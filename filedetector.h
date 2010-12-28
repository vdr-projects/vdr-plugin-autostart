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
#include "extstring.h"


class cFileDetector : public cMediaTester {
public:
    cFileDetector(cLogger *l, std::string descr, std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
        mConfiguredAutoMount = true;
    }

    bool isMedia (const cMediaHandle d, cExtStringVector &keylist);
    cMediaTester *create(cLogger *l) const {
        return new cFileDetector(l, mDescription, mExt);
    }
    bool loadConfig (cConfigFileParser config,
                       const cExtString sectionname);
    void startScan (cMediaHandle &d);
    void endScan (cMediaHandle &d);
    void removeDevice (cMediaHandle d);

private:
    typedef struct {
        cExtString devPath;
        cExtString linkPath;
    } DEVINFO;
    typedef std::set<cExtString> StringSet;
    typedef std::map<cExtString, DEVINFO> DevMap;

    static StringSet mDetectedSuffixCache;
    // Devices which are already processed
    static DevMap mDeviceMap;
    static std::string mLinkPath;
    static bool mAutoMount;

    StringSet mSuffix;
    cExtString mConfiguredLinkPath;
    bool mConfiguredAutoMount;


    void ClearSuffixCache (void) {mDetectedSuffixCache.clear();}
    bool FindSuffix (const cExtString str);
    std::string GetSuffix (const std::string str);
    void BuildSuffixCache (std::string path);
    bool inDeviceSet(const cExtString dev) {
        return (mDeviceMap.find(dev) != mDeviceMap.end());
    }
    bool RmLink(const cExtString ln);
    void Link(const cExtString ln);
};

#endif /* FILEDETECTOR_H_ */
