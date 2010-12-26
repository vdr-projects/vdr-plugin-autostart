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

#include <string>
#include <set>
#include <map>
#include "mediatester.h"
#include "extstring.h"


class cFileDetector : public cMediaTester {
public:
    cFileDetector(cLogger l, std::string descr, std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
        mConfiguredMountProg = "/bin/mount";
    }

    bool isMedia (const cMediaHandle d, cExtStringVector &keylist);
    cMediaTester *create(cLogger l) const {
        return new cFileDetector(l, mDescription, mExt);
    }
    bool loadConfig (cConfigFileParser config,
                       const cExtString sectionname);
    void startScan (cMediaHandle d);
    void endScan (cMediaHandle d);
    void removeDevice (cMediaHandle d);

private:
    typedef std::set<cExtString> StringSet;
    typedef std::map<cExtString, cExtString> StringMap;
    static StringSet mSuffix;
    static StringSet mDetectedSuffixCache;
    // Devices which are already processed
    static StringMap mDeviceMap;
    static std::string mMountPath;
    static std::string mMountProg;

    cExtString mConfiguredMountPath;
    cExtString mConfiguredMountProg;

    void ClearSuffixCache (void) {mDetectedSuffixCache.clear();}
    bool FindSuffix (const cExtString str);
    std::string GetSuffix (const std::string str);
    void BuildSuffixCache (std::string path);
    bool inDeviceSet(const cExtString dev) {
        return (mDeviceMap.find(dev) != mDeviceMap.end());
    }
    void Mount(const cExtString dev) {
        std::string cmd = mMountProg + " \"" + dev + "\" \"" + mMountPath + "\"";
        mLogger.logmsg(LOGLEVEL_INFO, cmd.c_str());
        system (cmd.c_str());
    }
};

#endif /* FILEDETECTOR_H_ */
