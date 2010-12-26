/*
 * mediadetector.cc
 *
 *  Created on: 17.10.2010
 *      Author: uli
 */

#include <string>
#include <stdlib.h>
#include "mediadetector.h"

using namespace std;

cMediaDetector::~cMediaDetector()
{
    MediaTesterVector::iterator it;
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        delete *it;
    }
    for (it = mRegisteredMediaTesters.begin(); it != mRegisteredMediaTesters.end(); it++) {
        delete *it;
    }
}

void cMediaDetector::RegisterTester(const cMediaTester *tester,
                                        const cConfigFileParser &config,
                                        const cExtString sectionname)
{
    cMediaTester *t = tester->create(mLogger);
    if (!t->loadConfig (mConfigFileParser, sectionname)) {
        mLogger.logmsg(LOGLEVEL_ERROR, "Error on parsing %s", sectionname.c_str());
        exit(-1);
    }
    mRegisteredMediaTesters.push_back(t);
}

bool cMediaDetector::AddDetector (const cExtString section)
{
    bool found = false;
    cExtStringVector vals;
    mLogger.logmsg(LOGLEVEL_INFO, "Section %s", section.c_str());
    cExtString type;
    if (!mConfigFileParser.GetSingleValue(section, "TYPE", type)) {
        mLogger.logmsg(LOGLEVEL_ERROR, "No type specified for section %s", section.c_str());
        return false;
    }

    MediaTesterVector::iterator it;
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *te = *it;
        if (te->typeMatches(type)) {
            RegisterTester(te, mConfigFileParser, section);
            found = true;
        }
    }

    if (!found) {
        mLogger.logmsg(LOGLEVEL_ERROR, "Invalid type %s", type.c_str());
    }
    return found;
}

bool cMediaDetector::AddGlobalOptions (const cExtString sectionname)
{
    cExtStringVector vals;

printf("Section %s\n", sectionname.c_str());
    if (sectionname != "GLOBAL") {
        return false;
    }

    if (mConfigFileParser.GetValues(sectionname, "FILTERDEV", vals)) {
        cExtStringVector::iterator it;
        for (it = vals.begin(); it != vals.end(); it++) {
            cExtString dev = *it;
            mLogger.logmsg(LOGLEVEL_INFO, "Filter dev %s", dev.c_str());
            mFilterDev.insert(dev);
        }
    }
    return true;
}

bool cMediaDetector::InitDetector(cLogger logger,
                                      const cExtString initfile)
{
    Section::iterator iter;
    cExtString sectionname;

    mLogger = logger;
    // Initialize known media testers
    mMediaTesters.push_back(new cCdioTester(mLogger, "Audio CD", "CD"));
    mMediaTesters.push_back(new cVideoDVDTester(mLogger, "Video DVD", "DVD"));
    mMediaTesters.push_back(new cFileDetector(mLogger, "Files", "FILE"));

    if (!mConfigFileParser.Parse(initfile)) {
        return false;
    }

    if (!mConfigFileParser.GetFirstSection(iter, sectionname)) {
        mLogger.logmsg(LOGLEVEL_ERROR, "Empty file");
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

bool cMediaDetector::InDeviceFilter(const string dev)
{
    FilterMap::iterator it;
    for (it = mFilterDev.begin(); it != mFilterDev.end(); it++) {
        string s = *it;
        if (dev.find(s) != string::npos) {
            return true;
        }
    }
    return false;
}

bool cMediaDetector::DoDetect(const cMediaHandle mediainfo,
                                  string &description,  cExtStringVector &vl)
{
    cExtStringVector keylist;
    bool found = false;
    MediaTesterVector::iterator it;

    // Initialize scan for each detector, e.g. the file detector will
    // build its cache.
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            t->startScan(mediainfo);
        } catch (cDeviceKitException &e) {
            mLogger.logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    // Do scan
    for (it = mRegisteredMediaTesters.begin();
         it != mRegisteredMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            if (t->isMedia(mediainfo, keylist)) {
                mLogger.logmsg(LOGLEVEL_INFO, "Found %s",
                        t->GetDescription().c_str());
#ifdef DEBUG
                logkeylist(keylist);
#endif
                description = t->GetDescription();
                found = true;
                break;
            }
        } catch (cDeviceKitException &e) {
            mLogger.logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    // Cleanup caches for each detector
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            t->endScan(mediainfo);
        } catch (cDeviceKitException &e) {
            mLogger.logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }

    if (found) {
        vl = keylist;
    }
    return found;
}

void cMediaDetector::DoDeviceRemoved(const cMediaHandle mediainfo)
{
    MediaTesterVector::iterator it;
    // Cleanup device caches for each detector
    for (it = mMediaTesters.begin(); it != mMediaTesters.end(); it++) {
        cMediaTester *t = *it;
        try {
            t->removeDevice(mediainfo);
        } catch (cDeviceKitException &e) {
            mLogger.logmsg(LOGLEVEL_INFO, "DeviceKit Error %s", e.what());
        }
    }
}

cExtStringVector cMediaDetector::Detect(string &description, cMediaHandle &mediainfo)
{
    string path;
    cMediaHandle descr;
    cExtStringVector keylist;
    cDbusDevkit::DEVICE_SIGNAL signal;
    running = true;
    while (running) {
        if (mDevkit.WaitDevkit(1000, path, signal)) {
            if (signal == cDbusDevkit::DeviceRemoved) {
                DoDeviceRemoved (mediainfo);
            } else {
                try {
                    descr.GetDescription(mDevkit, path);

#ifdef DEBUG
                    mLogger.logmsg(LOGLEVEL_INFO, "Path       : %s",
                            path.c_str());
                    mLogger.logmsg(LOGLEVEL_INFO, "NativePath : %s",
                            mDevkit.GetNativePath(path).c_str());
                    mLogger.logmsg(LOGLEVEL_INFO, "Type       : %s",
                            mDevkit.GetType(path).c_str());
                    mLogger.logmsg(LOGLEVEL_INFO, "Device File: %s",
                            mDevkit.GetDeviceFile(path).c_str());

                    if (mDevkit.IsMounted(path)) {
                        mLogger.logmsg(LOGLEVEL_INFO, "Mounted -----> %s",
                                mDevkit.GetMountPaths(path).at(0).c_str());
                    }
                    if (mDevkit.IsOpticalDisk(path)) {
                        mLogger.logmsg(LOGLEVEL_INFO, "Optical Disk ");
                    }
                    if (mDevkit.IsPartition(path)) {
                        mLogger.logmsg(LOGLEVEL_INFO, "Partition ");
                    }
                    if (mDevkit.IsMediaAvailable(path)) {
                        mLogger.logmsg(LOGLEVEL_INFO, "Media Available ");
                    }
#endif
                } catch (cDeviceKitException &e) {
                    mLogger.logmsg(LOGLEVEL_WARNING, "DeviceKit Error %s",
                            e.what());
                }

                if (InDeviceFilter(mDevkit.GetDeviceFile(path))) {
                    mLogger.logmsg(LOGLEVEL_INFO, "Device %s in device filter",
                            mDevkit.GetDeviceFile(path).c_str());
                } else {
#ifdef DEBUG
                    mLogger.logmsg(LOGLEVEL_INFO, "  ******** Detect ********");
#endif
                    if (DoDetect(descr, description, keylist)) {
                        mediainfo = descr;
                        return (keylist);
                    }
                }
            }
        }
    }
    keylist.clear();
    return (keylist);
}
