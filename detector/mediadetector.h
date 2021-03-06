/*
 * mediatdetecor.h: Main media detection class which initialize all media
 *                  testers, waits for a media change and try to detect
 *                  the inserted media.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef MEDIADETECTOR_H_
#define MEDIADETECTOR_H_

#include "configfileparser.h"
#include "filetester.h"
#include "cdiotester.h"
#include "videodvdtester.h"
#include "logger.h"
#include "stdtypes.h"


class cMediaDetector {
public:
    typedef enum {
        AUTO_START,
        MANUAL_START,
        LAST_MODE
    } WORKING_MODE;

    cMediaDetector(cLogger *l) : mConfigFileParser(l), mDevkit(l) {
        mRunning = false;
        mWorkingMode = AUTO_START;
        mManualScan = false;
        mManualFilterDevice = false;
        mLogger = NULL;
    }

    ~cMediaDetector();
    bool InitDetector(cLogger *logger, const std::string initfile);
    // Stop detector
    void Stop(void) { mRunning = false; };
    // Wait for a media change, detect the media and return the associated
    // key list and media information.
    stringList Detect(std::string &description, cMediaHandle &mediainfo);
    // Change working mode
    void SetWorkingMode (WORKING_MODE mode) {mWorkingMode = mode;}
    void StartManualScan (void) { mManualScan = true; };

private:
  //  typedef std::map<std::string, stringList> PluginMap;
    typedef std::list<cMediaTester *> MediaTesterList;
    typedef std::set<std::string> stringSet;

    cLogger *mLogger;
    cConfigFileParser mConfigFileParser;

    MediaTesterList mRegisteredMediaTesters;
    MediaTesterList mMediaTesters;
   // PluginMap mPlugins;

    cDbusDevkit mDevkit;

    WORKING_MODE mWorkingMode;
    // Devices in filter list
    stringSet mFilterDevices;
    stringSet mAutoFilterDevices;

    // Devices which are known due to insertion of a removable media
    stringSet mKnownDevices;
    // Devices to always scan, when manual scan is started
    stringSet mScanDevices;
    // Filterdevices specified manually
    bool mManualFilterDevice;

    volatile bool mRunning;
    volatile bool mManualScan;

    bool AddDetector(const std::string plugin);
    bool AddGlobalOptions(const std::string sectionname);
    void RegisterTester(const cMediaTester *, const cConfigFileParser &,
                          const std::string);
    bool InDeviceFilter(const std::string dev);
    bool DoDetect(cMediaHandle &, std::string &, stringList &);
    bool DoManualScan(cMediaHandle &, std::string &, stringList &);
    void DoDeviceRemoved(cMediaHandle mediainfo);

    void ParseFstab (stringList &values);

#ifdef DEBUG
    void logkeylist (stringList vl) {
        stringList::iterator it;
        for (it = vl.begin(); it != vl.end(); it++) {
            mLogger->logmsg(LOGLEVEL_INFO, "   %s", it->c_str());
        }
    }
#endif

};
#endif /* MEDIADETECTOR_H_ */
