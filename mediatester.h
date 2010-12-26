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
#include "extstring.h"
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

class cMediaHandle
{
private:
    cExtString mPath;
    cExtString mNativePath;
    cExtString mDeviceFile;
    cExtString mType;
    MEDIA_MASK_T mMediaMask;
    cDbusDevkit *mDevKit;
    cLogger mLogger;
    bool mAutomounted;

public:
    cMediaHandle() {};
    bool GetDescription(cDbusDevkit &d, const std::string &path);
    cExtString GetNativePath(void) {return mNativePath;}
    cExtString GetDeviceFile(void) {return mDeviceFile;}
    cExtString GetType(void) {return (mType);}
    cExtString GetPath(void) {return (mPath);}
    MEDIA_MASK_T GetMediaMask(void) {return (mMediaMask);}
    bool AutoMount(cExtString &mountpath);
    void Umount(void);
    bool isAutoMounted (void) {return mAutomounted;}
};

class cMediaTester
{
protected:
    cLogger mLogger;
    cExtStringVector mKeylist;
    std::string mDescription;
    std::string mExt;

public:
    virtual bool loadConfig (cConfigFileParser config,
                                const cExtString sectionname);
    virtual bool isMedia (cMediaHandle d, cExtStringVector &keylist) = 0;
    virtual cMediaTester *create(cLogger) const = 0;
    bool typeMatches (const cExtString name) const;
    virtual void startScan (cMediaHandle d) {};
    virtual void endScan (cMediaHandle d) {};
    virtual void removeDevice (cMediaHandle d) {};
    cExtStringVector getList (cConfigFileParser config,
                        const cExtString sectionname,
                        const cExtString key);
    std::string GetDescription(void) {return mDescription;}
};

#endif /* MEDIATESTER_H_ */
