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
#include "mediatester.h"
#include "extstring.h"


class cFileDetector : public cMediaTester {
public:
    cFileDetector(cLogger l, std::string descr, std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
    }

    bool isMedia (const cMediaHandle d, ValueList &keylist);
    cMediaTester *create(cLogger l) const {
        return new cFileDetector(l, mDescription, mExt);
    }
    bool loadConfig (cConfigFileParser config,
                       const cExtString sectionname);
    void startScan (cMediaHandle d);
    void endScan (cMediaHandle d);

private:
    void ClearSuffixCache (void) {mDetectedSuffixCache.clear();}
    bool FindSuffix (const cExtString str);
    std::string GetSuffix (const std::string str);
    void BuildSuffixCache (std::string path);

    typedef std::set<cExtString> SuffixType;
    static SuffixType suffix;
    static SuffixType mDetectedSuffixCache;
};

#endif /* FILEDETECTOR_H_ */
