/*
 * filedetetor.h: Detects file types and returns a list of keys depending on
 *                the detected file type.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef FILEDETECTOR_H_
#define FILEDETECTOR_H_

#include <unistd.h>
#include <string>
#include <set>
#include <map>
#include "mediatester.h"
#include "stringtools.h"


class cFileDetector : public cMediaTester {
public:
    cFileDetector(cLogger *l, std::string descr, std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
        mConfiguredAutoMount = true;
    }

    bool isMedia (const cMediaHandle d, stringVector &keylist);
    cMediaTester *create(cLogger *l) const {
        return new cFileDetector(l, mDescription, mExt);
    }
    bool loadConfig (cConfigFileParser config,
                       const std::string sectionname);
    void startScan (cMediaHandle &d);
    void endScan (cMediaHandle &d);
    void removeDevice (cMediaHandle d);

private:
    cFileDetector() {};
    typedef struct {
        std::string devPath;
        std::string linkPath;
    } DEVINFO;
    typedef std::set<std::string> stringSet;
    typedef std::map<std::string, DEVINFO> DevMap;

    static stringSet mDetectedSuffixCache;
    // Devices which are already processed
    static DevMap mDeviceMap;
    static std::string mLinkPath;
    static bool mAutoMount;

    stringSet mSuffix;
    std::string mConfiguredLinkPath;
    bool mConfiguredAutoMount;


    void ClearSuffixCache (void) {mDetectedSuffixCache.clear();}
    bool FindSuffix (const std::string str);
    std::string GetSuffix (const std::string str);
    void BuildSuffixCache (std::string path);
    bool inDeviceSet(const std::string dev) {
        return (mDeviceMap.find(dev) != mDeviceMap.end());
    }
    bool RmLink(const std::string ln);
    void Link(const std::string ln);
};

#endif /* FILEDETECTOR_H_ */
