/*
 * mediatdetecor.h: Main media detection class which initialize all media
 *                  detectors, waits for a media change and try to detect
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
private:
    typedef std::map<std::string, cExtStringVector> PluginMap;
    typedef std::vector<cMediaTester *> MediaTesterVector;
    typedef std::set<std::string> FilterMap;

    MediaTesterVector mRegisteredMediaTesters;
    PluginMap mPlugins;
    cConfigFileParser mConfigFileParser;
    cDbusDevkit mDevkit;
    cLogger mLogger;
    MediaTesterVector mMediaTesters;
    volatile bool running;

    FilterMap mFilterDev;
    bool AddDetector(const cExtString plugin);
    bool AddGlobalOptions(const cExtString sectionname);
    void RegisterTester(const cMediaTester *, const cConfigFileParser &,
                          const cExtString);
    bool InDeviceFilter(const std::string dev);
    bool DoDetect(const cMediaHandle, std::string &, cExtStringVector &);
    void DoDeviceRemoved(const cMediaHandle mediainfo);
#ifdef DEBUG
    void logkeylist (cExtStringVector vl) {
        cExtStringVector::iterator it;
        for (it = vl.begin(); it != vl.end(); it++) {
            mLogger.logmsg(LOGLEVEL_INFO, "   %s", it->c_str());
        }
    }
#endif

public:
    cMediaDetector() {running = false; };
    ~cMediaDetector();
    bool InitDetector(cLogger logger, const cExtString initfile);
    void Stop(void) { running = false; };
    // Wait for a media change, detect the media and return the associated
    // key list and media information.
    cExtStringVector Detect(std::string &description, cMediaHandle &mediainfo);
};
#endif /* MEDIADETECTOR_H_ */
