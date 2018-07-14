/*
 * filedetector.cc: Detects file types and returns a list of keys depending on
 *                  the detected file type.
 *
 * Copyright (C) 2010-2018 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
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

#include "filetester.h"

using namespace std;

cFileTester::stringSet cFileTester::mDetectedSuffixCache;
cFileTester::DevMap cFileTester::mDeviceMap;
string cFileTester::mLinkPath;
bool cFileTester::mAutoMount = true;

bool cFileTester::RmLink(const string ln)
{
    struct stat s;
    const char *to = ln.c_str();

    if (lstat(to, &s) != 0) {
        if (errno != ENOENT) {
            string err = strerror(errno);
            mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: Error removing %s: %s",
                            to, strerror(errno));
            return false;
        }
    }
    else {
        if (S_ISLNK(s.st_mode)) {
            if (unlink(to) != 0) {
                 mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: Can not remove %s: %s",
                                to, strerror(errno));
            return false;
            }
        }
        else {
            mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: File/Directory %s already exist",
                           to);
            return false;
        }
    }
    return true;
}

void cFileTester::Link(const string ln)
{
    const char *from = ln.c_str();
    const char *to = mLinkPath.c_str();

    if (!RmLink(to)) {
        return;
    }
    if (symlink(from, to) != 0) {
        string err = strerror(errno);
        mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: Can not link from %s to %s: %s", from,
                        to, strerror(errno));
    }
}

bool cFileTester::FindSuffix (const string str) {
    if (mSuffix.find(str) == mSuffix.end()) {
        return (false);
    }
    return (true);
}

string cFileTester::GetSuffix (const string str) {
    size_t pos = 0;
    pos = str.find_last_of('.');

    if (pos == string::npos) {
        return "";
    }

    return str.substr(pos+1);
}

void cFileTester::BuildSuffixCache (string path) {
    DIR *dp;
    struct dirent *ep;
    struct stat st;
    string file;

    if (path.empty()) {
        mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: No mount path");
        return;
    }

    if (path.back() != '/') {
        path.append("/");
    }
#ifdef DEBUG
    mLogger->logmsg(LOGLEVEL_ERROR, "file/dir %s", path.c_str());
#endif
    dp = opendir (path.c_str());
    if (dp == NULL) {
        mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: Could not open %s : %s",
                       path.c_str(), strerror (errno));
        return;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_name[0] != '.') {
            file = path + ep->d_name;

            if (stat(file.c_str(), &st) != 0) {
                mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: Can not stat file %s: %s",
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

bool cFileTester::isMedia (cMediaHandle d, stringList &keylist)
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

bool cFileTester::loadConfig (cConfigFileParser config,
                                  const string sectionname)
{
    if (!cMediaTester::loadConfig(config, sectionname)) {
        return false;
    }

    stringList vals;
    if (!config.GetValues(sectionname, "FILES", vals)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: No files specified");
        return false;
    }

    stringList::iterator it;
    for (it = vals.begin(); it != vals.end(); it++) {
        string s = *it;
        mSuffix.insert(s);
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester:  Add file %s", s.c_str());
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
            mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: Invalid keyword %s for AUTOMOUNT",
                                            automount.c_str());
            return false;
        }
    }

    return true;
}

void cFileTester::startScan (cMediaHandle &d, cDbusDevkit *devkit)
{
    MEDIA_MASK_T m = d.GetMediaMask();
    string dev = d.GetDeviceFile();

    mDevKit = devkit;
    mLinkPath.clear();
    mDetectedSuffixCache.clear();
    if (!(m & MEDIA_AVAILABLE))
    {
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Remove from device set");
        removeDevice (d);
        return;
    }

    if (inDeviceSet(dev)) {
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: startScan Device already in device set");
        return;
    }
    if (!AutoMount(d.GetPath())) {
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Automount failed");
        mMountError = true;
        return;
    }
    mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Build cache for device %s", dev.c_str());
    BuildSuffixCache(GetMountPath());
}

void cFileTester::endScan (cMediaHandle &d)
{
    string dev = d.GetDeviceFile();

    if (hasMountError()) {
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester:: error on scan");
        return;
    }
    if (inDeviceSet(dev)) {
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: endScan Device already in device set");
        return;
    }
    if ((isAutoMounted()) && (!mLinkPath.empty())) {
        mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Linking to %s", mLinkPath.c_str());
        Link(GetMountPath());
    }
    if (!mAutoMount) {
        Umount(dev);
    }
    DEVINFO devinfo;
    devinfo.devPath = d.GetPath();
    devinfo.linkPath = mLinkPath;
    mDeviceMap[dev] = devinfo;
    mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Add Device %s to device set", dev.c_str());
}

void cFileTester::removeDevice (cMediaHandle d)
{
    string path = d.GetPath();
    mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Removing %s", path.c_str());
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

// Try to auto mount the media
bool cFileTester::AutoMount(string devpath)
{
    int i;
    mMountPath.clear();
    // We will try several times since other tasks, for example
    // other window managers may also try to automount.
    for(i = 0; i < 3; i++)
    {
        try {
            if (!mDevKit->IsMounted(devpath)) {
                mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Try to AutoMount : %s",
                        mDevKit->AutoMount(devpath).c_str());
            }
            mMountPath = mDevKit->GetMountPaths(devpath).front();
#ifdef DEBUG
            mLogger->logmsg(LOGLEVEL_INFO, "cFileTester: Mount Path >%s< %d",
                    mMountPath.c_str(), mMountPath.length());
#endif
            if (!mMountPath.empty()) {
                return true;
            }
        } catch (cDeviceKitException &e) {
#ifdef DEBUG
            // Error No such interface 'org.freedesktop.UDisks2.Filesystem' is OK for medias without
            // a file system eg audio CDs
            mLogger->logmsg(LOGLEVEL_ERROR, "cFileTester: AutoMount DeviceKit Error %s", e.what());
#endif
            return false;
        }
        sleep(1);
    }
    return false;
}

void cFileTester::Umount(string devpath)
{
    mDevKit->UnMount(devpath);
    mMountPath.clear();
}

