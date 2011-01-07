/*
 * mediadetector.cc: Main detector loop and initialization of
 *                   known media testers.
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */
#include <string>
#include <stdlib.h>
#include "mediadetector.h"
#include <assert.h>

using namespace std;

cMediaDetector::~cMediaDetector()
{
    MediaTesterList::iterator it;
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        delete *it;
    }
    for (it = mRegisteredMediaTesters.begin(); it != mRegisteredMediaTesters.end(); it++) {
        delete *it;
    }
}

// Create a new instance for a tester, initialize this instance with the contens
// of the config file and register this tester, for example
// the file detector reads in the suffix list associated with a file detector
// instance.
void cMediaDetector::RegisterTester(const cMediaTester *tester,
                                        const cConfigFileParser &config,
                                        const string sectionname)
{
    cMediaTester *t = tester->create(mLogger);
    if (!t->loadConfig (mConfigFileParser, sectionname)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Error on parsing %s", sectionname.c_str());
        exit(-1);
    }
    mRegisteredMediaTesters.push_back(t);
}

// Search the corresponding tester for the given TYPE keyword in the
// config file and register an instance of this detector.
bool cMediaDetector::AddDetector (const string section)
{
    bool found = false;
    stringList vals;
    mLogger->logmsg(LOGLEVEL_INFO, "Section %s", section.c_str());
    string type;
    if (!mConfigFileParser.GetSingleValue(section, "TYPE", type)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "No type specified for section %s", section.c_str());
        return false;
    }

    MediaTesterList::iterator it;
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *te = *it;
        if (te->typeMatches(type)) {
            RegisterTester(te, mConfigFileParser, section);
            found = true;
        }
    }

    if (!found) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Invalid type %s", type.c_str());
    }
    return found;
}

// Read global options for the plugin
bool cMediaDetector::AddGlobalOptions (const string sectionname)
{
    stringList vals;
    stringList::iterator it;
    string dev;

    if (sectionname != "GLOBAL") {
        return false;
    }

    if (mConfigFileParser.GetValues(sectionname, "FILTERDEV", vals)) {
        for (it = vals.begin(); it != vals.end(); it++) {
            dev = *it;
            mLogger->logmsg(LOGLEVEL_INFO, "Filter dev %s", dev.c_str());
            mFilterDevices.insert(dev);
        }
    }
    if (mConfigFileParser.GetValues(sectionname, "SCANDEV", vals)) {
        for (it = vals.begin(); it != vals.end(); it++) {
            dev = *it;
            if (dev[0] != '/') {
                dev = "/dev/" + dev;
            }
            mLogger->logmsg(LOGLEVEL_INFO, "Scan dev %s", dev.c_str());
            mScanDevices.insert(dev);

#ifdef DEBUG
            try {
                mLogger->logmsg(LOGLEVEL_INFO, "Path : %s",
                        mDevkit.FindDeviceByDeviceFile(dev).c_str());
            } catch (cDeviceKitException &e) {
                mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
            }
#endif
        }
    }

    return true;
}

// Initialize the detector and create a first instance of each known
// media tester.
bool cMediaDetector::InitDetector(cLogger *logger,
                                      const string initfile)
{
    Section::iterator iter;
    string sectionname;

    mLogger = logger;
    // Initialize known media testers
cCdioTester *t = new cCdioTester(logger, "Audio CD", "CD");
mRegisteredMediaTesters.clear();
mMediaTesters.clear();
    mMediaTesters.push_back(t);
    mMediaTesters.push_back(new cVideoDVDTester(logger, "Video DVD", "DVD"));
    mMediaTesters.push_back(new cFileDetector(logger, "Files", "FILE"));

    if (!mConfigFileParser.Parse(initfile)) {
        return false;
    }

    if (!mConfigFileParser.GetFirstSection(iter, sectionname)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Empty file");
        return false;
    }

    if (!AddGlobalOptions(sectionname)) {
        if (!AddDetector(sectionname)) {
            return false;
        }
    }

    while (mConfigFileParser.GetNextSection(iter, sectionname)) {
        if (!AddGlobalOptions(sectionname)) {
            if (!AddDetector(sectionname)) {
                return false;
            }
        }
    }
    return true;
}

// Helper function to check if a device is in the exclude filter
bool cMediaDetector::InDeviceFilter(const string dev)
{
    stringSet::iterator it;
    for (it = mFilterDevices.begin(); it != mFilterDevices.end(); it++) {
        string s = *it;
        if (dev.find(s) != string::npos) {
            return true;
        }
    }
    return false;
}

// Handle when a device is removed
void cMediaDetector::DoDeviceRemoved(cMediaHandle mediainfo)
{
    MediaTesterList::iterator it;
    // Cleanup device caches for each detector
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            t->removeDevice(mediainfo);
        } catch (cDeviceKitException &e) {
            mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }
    mKnownDevices.erase(mediainfo.GetDeviceFile());
}

bool cMediaDetector::DoManualScan(cMediaHandle &mediainfo,
                                  string &description,  stringList &vl)
{
    stringSet::iterator it;
    for (it = mKnownDevices.begin(); it != mKnownDevices.end(); it++) {
        string path = *it;
        mLogger->logmsg(LOGLEVEL_INFO, "Manual Scan %s", path.c_str());
        try {
            mediainfo.GetDescription(mDevkit, path);
            if (DoDetect (mediainfo, description, vl)) {
                return true;
            }
        } catch (cDeviceKitException &e) {
            mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }
    for (it = mScanDevices.begin(); it != mScanDevices.end(); it++) {
        string path = *it;
        mLogger->logmsg(LOGLEVEL_INFO, "Manual Scan %s", path.c_str());
        try {
            mediainfo.GetDescription(mDevkit, path);
            if (DoDetect (mediainfo, description, vl)) {
                return true;
            }
        } catch (cDeviceKitException &e) {
            mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    return false;
}

bool cMediaDetector::DoDetect(cMediaHandle &mediainfo,
                                  string &description,  stringList &vl)
{
    stringList keylist;
    bool found = false;
    MediaTesterList::iterator it;
    string dev = mediainfo.GetDeviceFile();
    if (!mManualScan) {
        if (mScanDevices.find(dev) == mScanDevices.end()) {
            mKnownDevices.insert(dev);
        }
        if (mWorkingMode == MANUAL_START) {
            return (false);
        }
    }
    // Initialize scan for each detector, e.g. the file detector will
    // build its cache.
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            t->startScan(mediainfo);
        } catch (cDeviceKitException &e) {
            mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    // Do scan
    for (it = mRegisteredMediaTesters.begin();
         it != mRegisteredMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            if (t->isMedia(mediainfo, keylist)) {
                mLogger->logmsg(LOGLEVEL_INFO, "Found %s",
                        t->GetDescription().c_str());
#ifdef DEBUG
                logkeylist(keylist);
#endif
                description = t->GetDescription();
                found = true;
                break;
            }
        } catch (cDeviceKitException &e) {
            mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    // Cleanup caches for each detector
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            t->endScan(mediainfo);
        } catch (cDeviceKitException &e) {
            mLogger->logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    if (found) {
        vl = keylist;
    }
    return found;
}

stringList cMediaDetector::Detect(string &description,
                                           cMediaHandle &mediainfo)
{
    string path;
    cMediaHandle descr(mLogger);
    stringList keylist;
    cDbusDevkit::DEVICE_SIGNAL signal;
    mRunning = true;
    mManualScan = false;
    while (mRunning) {
        // Wait until device kit detects a media change
        if (mDevkit.WaitDevkit(1000, path, signal)) {
            // A removed device needs special handling
            if (signal == cDbusDevkit::DeviceRemoved) {
                DoDeviceRemoved (mediainfo);
            } else {
                try {
                    descr.GetDescription(mDevkit, path);

#ifdef DEBUG
                    mLogger->logmsg(LOGLEVEL_INFO, "Path       : %s",
                            path.c_str());
                    mLogger->logmsg(LOGLEVEL_INFO, "NativePath : %s",
                            mDevkit.GetNativePath(path).c_str());
                    mLogger->logmsg(LOGLEVEL_INFO, "Type       : %s",
                            mDevkit.GetType(path).c_str());
                    mLogger->logmsg(LOGLEVEL_INFO, "Device File: %s",
                            mDevkit.GetDeviceFile(path).c_str());

                    if (mDevkit.IsMounted(path)) {
                        mLogger->logmsg(LOGLEVEL_INFO, "Mounted -----> %s",
                                mDevkit.GetMountPaths(path).front().c_str());
                    }
                    if (mDevkit.IsOpticalDisk(path)) {
                        mLogger->logmsg(LOGLEVEL_INFO, "Optical Disk ");
                    }
                    if (mDevkit.IsPartition(path)) {
                        mLogger->logmsg(LOGLEVEL_INFO, "Partition ");
                    }
                    if (mDevkit.IsMediaAvailable(path)) {
                        mLogger->logmsg(LOGLEVEL_INFO, "Media Available ");
                    }
#endif
                    if (InDeviceFilter(descr.GetDeviceFile())) {
                        mLogger->logmsg(LOGLEVEL_INFO,
                                "Device %s in device filter",
                                descr.GetDeviceFile().c_str());
                    } else {
#ifdef DEBUG
                        mLogger->logmsg(LOGLEVEL_INFO,
                                "  ******** Detect ********");
#endif
                        // Detect media and return keylist if detection was
                        // successful
                        if (DoDetect(descr, description, keylist)) {
                            mediainfo = descr;
                            return (keylist);
                        }
                    }
                } catch (cDeviceKitException &e) {
                    mLogger->logmsg(LOGLEVEL_WARNING, "DeviceKit Error %s",
                            e.what());
                }

            }
        }
        else { // Start manual scan
            if (mManualScan) {
                if (DoManualScan(descr, description, keylist)) {
                    mediainfo = descr;
                    return (keylist);
                }
                mManualScan = false;
            }
        }
    }
    keylist.clear();
    return (keylist);
}
