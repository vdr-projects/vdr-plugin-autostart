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

cFileDetector::StringSet cFileDetector::mSuffix;
cFileDetector::StringSet cFileDetector::mDetectedSuffixCache;
cFileDetector::StringMap cFileDetector::mDeviceMap;
string cFileDetector::mMountPath;
string cFileDetector::mMountProg;

bool cFileDetector::FindSuffix (const cExtString str) {
    StringSet::iterator iter;
    iter = mSuffix.find(str);

    if (iter == mSuffix.end()) {
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

bool cFileDetector::isMedia (cMediaHandle d, cExtStringVector &keylist)
{
    MEDIA_MASK_T m = d.GetMediaMask();
    bool found = false;
    cExtString mountpath;

    if (!(m & MEDIA_AVAILABLE)) {
        return false;
    }
    if (inDeviceSet(d.GetDeviceFile())) {
        return false;
    }
    if (!d.isAutoMounted()) {
        return false;
    }
    StringSet::iterator it;
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
        mMountPath = mConfiguredMountPath;
        mMountProg = mConfiguredMountProg;
    }
    return found;
}

bool cFileDetector::loadConfig (cConfigFileParser config,
                                    const cExtString sectionname)
{
    if (!cMediaTester::loadConfig(config, sectionname)) {
        return false;
    }

    cExtStringVector vals;
    if (!config.GetValues(sectionname, "FILES", vals)) {
        mLogger.logmsg(LOGLEVEL_ERROR, "No files specified");
        return false;
    }

    cExtStringVector::iterator it;
    for (it = vals.begin(); it != vals.end(); it++) {
        string s = *it;
        mSuffix.insert(s);
        mLogger.logmsg(LOGLEVEL_INFO, "  Add file %s", s.c_str());
    }

    // Read Mount-Path
    config.GetSingleValue(sectionname, "MOUNTPATH", mConfiguredMountPath);
    mLogger.logmsg(LOGLEVEL_INFO, "Mountpath : %s", mConfiguredMountPath.c_str());
    // Read Mount-Programm
    config.GetSingleValue(sectionname, "MOUNTPROG", mConfiguredMountProg);
    mLogger.logmsg(LOGLEVEL_INFO, "Mountprog : %s", mConfiguredMountProg.c_str());
    return true;
}

void cFileDetector::startScan (cMediaHandle d)
{
    MEDIA_MASK_T m = d.GetMediaMask();
    cExtString mountpath;
    cExtString dev = d.GetDeviceFile();

    mMountPath.clear();
    mDetectedSuffixCache.clear();
    if (!(m & MEDIA_AVAILABLE))
    {
        mLogger.logmsg(LOGLEVEL_INFO, "Remove from device set");
        mDeviceMap.erase(dev);
        return;
    }

    if (inDeviceSet(dev)) {
        mLogger.logmsg(LOGLEVEL_INFO, "Device already in device set");
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
    cExtString dev = d.GetDeviceFile();
    mDeviceMap[dev] = d.GetPath();
    if (!mMountPath.empty()) {
        d.Umount();
        mLogger.logmsg(LOGLEVEL_INFO, "Mounting to %s", mMountPath.c_str());
        Mount(dev);
    }
}

void cFileDetector::removeDevice (cMediaHandle d)
{
    cExtString path = d.GetPath();
    mLogger.logmsg(LOGLEVEL_INFO, "Removing %s", path.c_str());
    StringMap::iterator it;
    cExtString s;
    for (it = mDeviceMap.begin(); it != mDeviceMap.end(); it++) {
        if (it->second == path) {
            mLogger.logmsg(LOGLEVEL_INFO, "Found %s", it->first.c_str());
            mDeviceMap.erase (it);
            return;
        }
    }
}
