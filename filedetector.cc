/*
 * filedetector.cc: Detects file types and returns a list of keys depending on
 *                  the detected file type.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "filedetector.h"

using namespace std;

cFileDetector::SuffixType cFileDetector::suffix;
cFileDetector::SuffixType cFileDetector::mDetectedSuffixCache;

bool cFileDetector::FindSuffix (const cExtString str) {
    SuffixType::iterator iter;
    iter = suffix.find(str);

    if (iter == suffix.end()) {
        return (false);
    }
    return (true);
}

string cFileDetector::GetSuffix (const string str) {
    size_t pos = 0;
    pos = str.find_last_of('.');

    if (pos == string::npos) {
        return "";
    }

    return str.substr(pos+1);
}

void cFileDetector::BuildSuffixCache (string path) {
    DIR *dp;
    struct dirent *ep;
    struct stat st;
    string file;

    if (path.at(path.length()-1) != '/') {
        path += "/";
    }

    dp = opendir (path.c_str());
    if (dp == NULL) {
        perror ("Couldn't open the directory");
        return;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_name[0] != '.') {
            file = path + ep->d_name;
            if (stat(file.c_str(), &st) != 0) {
                mLogger.logmsg(LOGLEVEL_ERROR, "Can not stat file %s: %s",
                               file.c_str(), strerror(errno));
                break;
            }
            if (S_ISDIR(st.st_mode)) {
                BuildSuffixCache(file);

            } else if (S_ISREG(st.st_mode)) {
                string suf = GetSuffix(ep->d_name);
                mDetectedSuffixCache.insert(suf);

            }
        }
    }
    (void)closedir(dp);
}

bool cFileDetector::isMedia (cMediaHandle d, ValueList &keylist)
{
    MEDIA_MASK_T m = d.GetMediaMask();
    bool found = false;
    cExtString mountpath;

    if (!(m & MEDIA_AVAILABLE))
    {
        return false;
    }

    SuffixType::iterator it;
    cExtString s;
    for (it = mDetectedSuffixCache.begin(); it != mDetectedSuffixCache.end(); it++) {
        s = *it;
        if (FindSuffix(s)) {
            found = true;
            break;
        }
    }

    if (found) {
        keylist = mKeylist;
    }
    return found;
}

bool cFileDetector::loadConfig (cConfigFileParser config,
                                    const cExtString sectionname)
{
    if (!cMediaTester::loadConfig(config, sectionname)) {
        return false;
    }

    ValueList vals;
    if (!config.GetValues(sectionname, "FILES", vals)) {
        mLogger.logmsg(LOGLEVEL_ERROR, "No files specified");
        return false;
    }

    ValueList::iterator it;
    for (it = vals.begin(); it != vals.end(); it++) {
        string s = *it;
        suffix.insert(s);
        mLogger.logmsg(LOGLEVEL_INFO, "  Add file %s", s.c_str());
    }
    return true;
}

void cFileDetector::startScan (cMediaHandle d)
{
    MEDIA_MASK_T m = d.GetMediaMask();
    cExtString mountpath;

    mDetectedSuffixCache.clear();
    if (!(m & MEDIA_AVAILABLE))
    {
        return;
    }

    if (!d.AutoMount(mountpath)) {
        mLogger.logmsg(LOGLEVEL_INFO, "Automount failed");
        return;
    }
    BuildSuffixCache(mountpath);
}

void cFileDetector::endScan (cMediaHandle d)
{
    d.Umount();
}
