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

cFileDetector::stringSet cFileDetector::mDetectedSuffixCache;
cFileDetector::DevMap cFileDetector::mDeviceMap;
string cFileDetector::mLinkPath;
bool cFileDetector::mAutoMount = true;

bool cFileDetector::RmLink(const string ln)
{
    struct stat s;
    const char *to = ln.c_str();

    if (lstat(to, &s) != 0) {
        if (errno != ENOENT) {
            string err = strerror(errno);
            mLogger->logmsg(LOGLEVEL_ERROR, "Error removing %s: %s",
                    to, strerror(errno));
            return false;
        }
    }
    else {
        if (S_ISLNK(s.st_mode)) {
            if (unlink(to) != 0) {
                 mLogger->logmsg(LOGLEVEL_ERROR, "Can not remove %s: %s",
                                to, strerror(errno));
            return false;
            }
        }
        else {
            mLogger->logmsg(LOGLEVEL_ERROR, "File/Directory %s already exist",
                           to);
            return false;
        }
    }
    return true;
}

void cFileDetector::Link(const string ln)
{
    const char *from = ln.c_str();
    const char *to = mLinkPath.c_str();

    if (!RmLink(to)) {
        return;
    }
    if (symlink(from, to) != 0) {
        string err = strerror(errno);
        mLogger->logmsg(LOGLEVEL_ERROR, "Can not link from %s to %s: %s", from,
                to, strerror(errno));
    }
}

bool cFileDetector::FindSuffix (const string str) {
    if (mSuffix.find(str) == mSuffix.end()) {
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
        mLogger->logmsg(LOGLEVEL_ERROR, "Could not open %s : %s",
                       path.c_str(), strerror (errno));
        return;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_name[0] != '.') {
            file = path + ep->d_name;
            if (stat(file.c_str(), &st) != 0) {
                mLogger->logmsg(LOGLEVEL_ERROR, "Can not stat file %s: %s",
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

bool cFileDetector::isMedia (cMediaHandle d, stringList &keylist)
{
    bool found = false;
    string mountpath;

    if (mDetectedSuffixCache.empty()) {
        return false;
    }
    stringSet::iterator it;
    string s;
    for (it = mDetectedSuffixCache.begin(); it != mDetectedSuffixCache.end(); it++) {
        s = *it;
        if (FindSuffix(s)) {
            found = true;
            break;
        }
    }

    if (found) {
        keylist = mKeylist;
        mLinkPath = mConfiguredLinkPath;
        mAutoMount = mConfiguredAutoMount;
    }
    return found;
}

bool cFileDetector::loadConfig (cConfigFileParser config,
                                    const string sectionname)
{
    if (!cMediaTester::loadConfig(config, sectionname)) {
        return false;
    }

    stringList vals;
    if (!config.GetValues(sectionname, "FILES", vals)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "No files specified");
        return false;
    }

    stringList::iterator it;
    for (it = vals.begin(); it != vals.end(); it++) {
        string s = *it;
        mSuffix.insert(s);
        mLogger->logmsg(LOGLEVEL_INFO, "  Add file %s", s.c_str());
    }

    // Read Link-Path
    config.GetSingleValue(sectionname, "LINKPATH", mConfiguredLinkPath);
    mLogger->logmsg(LOGLEVEL_INFO, "Linkpath : %s", mConfiguredLinkPath.c_str());
    // Read Automount
    string automount;
    if (config.GetSingleValue(sectionname, "AUTOMOUNT", automount)) {
        automount = StringTools::ToUpper(automount);
        if (automount == "YES") {
            mConfiguredAutoMount = true;
        }
        else if (automount == "NO") {
            mConfiguredAutoMount = false;
        }
        else {
            mLogger->logmsg(LOGLEVEL_ERROR, "Invalid keyword %s for AUTOMOUNT",
                                            automount.c_str());
            return false;
        }
    }

    return true;
}

void cFileDetector::startScan (cMediaHandle &d)
{
    MEDIA_MASK_T m = d.GetMediaMask();
    string dev = d.GetDeviceFile();

    mLinkPath.clear();
    mDetectedSuffixCache.clear();
    if (!(m & MEDIA_AVAILABLE))
    {
        mLogger->logmsg(LOGLEVEL_INFO, "Remove from device set");
        mDeviceMap.erase(dev);
        return;
    }

    if (inDeviceSet(dev)) {
        mLogger->logmsg(LOGLEVEL_INFO, "startScan Device already in device set");
        return;
    }
    if (!d.AutoMount()) {
        mLogger->logmsg(LOGLEVEL_INFO, "Automount failed");
        return;
    }

    BuildSuffixCache(d.GetMountPath());
}

void cFileDetector::endScan (cMediaHandle &d)
{
    string dev = d.GetDeviceFile();
    if (inDeviceSet(dev)) {
        mLogger->logmsg(LOGLEVEL_INFO, "endScan Device already in device set");
        return;
    }

    if ((d.isAutoMounted()) && (!mLinkPath.empty())) {
        mLogger->logmsg(LOGLEVEL_INFO, "Linking to %s", mLinkPath.c_str());
        Link(d.GetMountPath());
    }
    if (!mAutoMount) {
        d.Umount();
    }
    DEVINFO devinfo;
    devinfo.devPath = d.GetPath();
    devinfo.linkPath = mLinkPath;
    mDeviceMap[dev] = devinfo;
}

void cFileDetector::removeDevice (cMediaHandle d)
{
    string path = d.GetPath();
    mLogger->logmsg(LOGLEVEL_INFO, "Removing %s", path.c_str());
    DevMap::iterator it;
    string s;
    for (it = mDeviceMap.begin(); it != mDeviceMap.end(); it++) {
        DEVINFO dev = it->second;
        if (dev.devPath == path) {
            mLogger->logmsg(LOGLEVEL_INFO, "Found %s", it->first.c_str());
            RmLink(dev.linkPath);
            mDeviceMap.erase (it);
            return;
        }
    }
}