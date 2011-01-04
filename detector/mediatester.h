/*
 * mediatester.h: Base class with base functionality for all media testers
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef MEDIATESTER_H_
#define MEDIATESTER_H_

#include "dbusdevkit.h"
#include "configfileparser.h"
#include "stringtools.h"
#include "logger.h"

typedef long MEDIA_MASK_T;

static const MEDIA_MASK_T MEDIA_OPTICAL    = 0x01;
static const MEDIA_MASK_T MEDIA_MOUNTED    = 0x02;
static const MEDIA_MASK_T MEDIA_PARTITON   = 0x04;
static const MEDIA_MASK_T MEDIA_FS_EXT2    = 0x08;
static const MEDIA_MASK_T MEDIA_FS_EXT3    = 0x10;
static const MEDIA_MASK_T MEDIA_FS_ISO9660 = 0x20;
static const MEDIA_MASK_T MEDIA_FS_UDF     = 0x40;
static const MEDIA_MASK_T MEDIA_FS_UNKNOWN = 0x80;
static const MEDIA_MASK_T MEDIA_AVAILABLE  = 0x100;
static const MEDIA_MASK_T MEDIA_FS_VFAT    = 0x200;

// This class holds information about the changed media including a
// reference to the dbus - devkit
class cMediaHandle
{
private:
    std::string mPath;
    std::string mNativePath;
    std::string mDeviceFile;
    std::string mType;
    std::string mMountPath;
    MEDIA_MASK_T mMediaMask;
    cDbusDevkit *mDevKit;
    cLogger *mLogger;

public:
    cMediaHandle() {mLogger = NULL; mDevKit = NULL;}
    cMediaHandle(cLogger *l) { mLogger = l; mDevKit = NULL; }
    bool GetDescription(cDbusDevkit &d, const std::string &path);
    std::string GetNativePath(void) {return mNativePath;}
    std::string GetDeviceFile(void) {return mDeviceFile;}
    std::string GetType(void) {return mType;}
    std::string GetPath(void) {return mPath;}
    std::string GetMountPath (void) {return mMountPath;}
    MEDIA_MASK_T GetMediaMask(void) {return mMediaMask;}
    bool AutoMount(void);
    void Umount(void);
    bool isAutoMounted (void) {return (!mMountPath.empty());}
};

class cMediaTester
{
protected:
    cLogger *mLogger;
    stringVector mKeylist;
    std::string mDescription;
    std::string mExt;

public:
    virtual bool loadConfig (cConfigFileParser config,
                                const std::string sectionname);
    virtual bool isMedia (cMediaHandle d, stringVector &keylist) = 0;
    virtual cMediaTester *create(cLogger *) const = 0;
    virtual void startScan (cMediaHandle &d) {};
    virtual void endScan (cMediaHandle &d) {};
    virtual void removeDevice (cMediaHandle d) {};
    virtual stringVector getList (cConfigFileParser config,
                                const std::string sectionname,
                                const std::string key);
    virtual bool typeMatches (const std::string name);
    virtual std::string GetDescription(void) {return mDescription;}
    virtual ~cMediaTester() {};
};

#endif /* MEDIATESTER_H_ */
