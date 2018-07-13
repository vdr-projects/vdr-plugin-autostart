/*
 * mediatester.cc: Base class with base functionality for all media testers
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include "mediatester.h"
#include <unistd.h>
#ifndef _NOVDR_
#include <vdr/plugin.h>
#else
#include "vdrsimulator.h"
#endif

using namespace std;

// Read media information from devkit/udisk
bool cMediaHandle::GetDescription (cDbusDevkit &d,
                                       const string &path)
{
    bool success = true;
    mPath = path;
    mDevKit = &d;
    try {
        mNativePath = d.GetNativePath(path);
        mDeviceFile = d.GetDeviceFile(path);
        mType = d.GetType(path);
        mMediaMask = 0;
        if (d.IsOpticalDisk(path)) {
            mMediaMask |= MEDIA_OPTICAL;
        }
        if (d.IsMounted(path)) {
            mMediaMask |= MEDIA_MOUNTED;
        }
        if (d.IsPartition(path)) {
            mMediaMask |= MEDIA_PARTITON;
        }
        if (d.IsMediaAvailable((path))) {
            mMediaMask |= MEDIA_AVAILABLE;
        }
        if (mType == "iso9660") {
            mMediaMask |= MEDIA_FS_ISO9660;
        }
        else if (mType == "udf") {
            mMediaMask |= MEDIA_FS_UDF;
        }
        else if (mType == "vfat") {
            mMediaMask |= MEDIA_FS_VFAT;
        }
        else if (mType.empty()) {
            mMediaMask |= MEDIA_FS_UNKNOWN;
        }
    }
    catch (cDeviceKitException &e) {
        mLogger->logmsg(LOGLEVEL_WARNING, "DeviceKit Error %s", e.what());
        success = false;
    }
    return (success);
}

stringList cMediaTester::getList(cConfigFileParser config,
                                    const string sectionname,
                                    const string key)
{
    stringList vals;
    if (!config.GetValues(sectionname, key, vals)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "No %s specified in section %s",
                key.c_str(), sectionname.c_str());
        exit(-1);
    }

    stringList::iterator it;
    for (it = vals.begin(); it != vals.end(); it++) {
        mLogger->logmsg(LOGLEVEL_INFO, "ADD %s %s", key.c_str(), it->c_str());
    }
    return vals;
}

bool cMediaTester::loadConfig (cConfigFileParser config,
                                   const string sectionname)
{
    if (!config.CheckSection (sectionname, mRequiredKeys, mOptionalKeys)) {
        return false;
    }
    // Each media tester must read in at least the KEYS keyword.
    mKeylist = getList (config, sectionname, "KEYS");
    if (mKeylist.empty()) {
        mLogger->logmsg(LOGLEVEL_ERROR, "No KEYS defined\n");
        return false;
    }
    stringList::iterator it;
    // Check input string
#ifndef _NOVDR_
    for (it = mKeylist.begin(); it != mKeylist.end(); it++) {
        string key = *it;
        mLogger->logmsg(LOGLEVEL_INFO, "Check key %s", key.c_str());
        if (key[0] == '@') {
            // Plugin
            key.erase(0,1);
            cPlugin *p = cPluginManager::GetPlugin(key.c_str());
            if (p == NULL) {
                mLogger->logmsg(LOGLEVEL_ERROR, "Plugin %s not available", key.c_str());
                return false;
            }
        }
        else {
            eKeys keycode = cKey::FromString(key.c_str());
            if (keycode == kNone) {
                mLogger->logmsg(LOGLEVEL_ERROR, "Unkown key %s", key.c_str());
                return false;
            }
        }
    }
#endif
    return true;
}

bool cMediaTester::typeMatches(const string name)
{
    string s = StringTools::ToUpper(name);
    return (s == mExt);
}
