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
#include "filedetector.h"
#include "cdiotester.h"
#include "videodvdtester.h"
#include "logger.h"


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
    }
    ~cMediaDetector();
    bool InitDetector(cLogger *logger, const cExtString initfile);
    // Stop detector
    void Stop(void) { mRunning = false; };
    // Wait for a media change, detect the media and return the associated
    // key list and media information.
    cExtStringVector Detect(std::string &description, cMediaHandle &mediainfo);
    // Change working mode
    void SetWorkingMode (WORKING_MODE mode) {mWorkingMode = mode;}
    void StartManualScan (void) { mManualScan = true; };
private:
    typedef std::map<std::string, cExtStringVector> PluginMap;
    typedef std::vector<cMediaTester *> MediaTesterVector;
    typedef std::set<std::string> StringSet;

    MediaTesterVector mRegisteredMediaTesters;
    PluginMap mPlugins;
    cConfigFileParser mConfigFileParser;
    cDbusDevkit mDevkit;
    cLogger *mLogger;
    MediaTesterVector mMediaTesters;
    WORKING_MODE mWorkingMode;
    // Devices in filter list
    StringSet mFilterDevices;
    // Devices which are known due to insertion of a removable media
    StringSet mKnownDevices;
    // Devices to alway scan, when manual scan is started
    StringSet mScanDevices;

    volatile bool mRunning;
    volatile bool mManualScan;

    bool AddDetector(const cExtString plugin);
    bool AddGlobalOptions(const cExtString sectionname);
    void RegisterTester(const cMediaTester *, const cConfigFileParser &,
                          const cExtString);
    bool InDeviceFilter(const std::string dev);
    bool DoDetect(cMediaHandle &, std::string &, cExtStringVector &);
    bool DoManualScan(cMediaHandle &, std::string &, cExtStringVector &);
    void DoDeviceRemoved(cMediaHandle mediainfo);

#ifdef DEBUG
    void logkeylist (cExtStringVector vl) {
        cExtStringVector::iterator it;
        for (it = vl.begin(); it != vl.end(); it++) {
            mLogger->logmsg(LOGLEVEL_INFO, "   %s", it->c_str());
        }
    }
#endif


};
#endif /* MEDIADETECTOR_H_ */
