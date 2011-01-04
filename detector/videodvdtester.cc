/*
 * videodvdtester.cc: Detects Video DVDs, using libdvdread.
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */
#include "videodvdtester.h"
#include <dvdread/dvd_reader.h>

bool cVideoDVDTester::isMedia (cMediaHandle d, stringVector &keylist)
{
    dvd_reader_t *reader;
    dvd_file_t *file;
    bool success = true;
    MEDIA_MASK_T m = d.GetMediaMask();

    if (!(m & MEDIA_OPTICAL)) {
        return (false);
    }
    if (!(m & MEDIA_AVAILABLE))
    {
        return (false);
    }
    if (!((m & MEDIA_FS_ISO9660) ||
          (m & MEDIA_FS_UDF) ||
          (m & MEDIA_FS_UNKNOWN)))
    {
        return (false);
    }
    reader = DVDOpen (d.GetDeviceFile().c_str());
    if (reader == NULL) {
        mLogger->logmsg(LOGLEVEL_INFO, "Can not open %s", d.GetDeviceFile().c_str());
        return false;
    }
    file = DVDOpenFile(reader, 0, DVD_READ_INFO_FILE );
    if (file == NULL) {
        mLogger->logmsg(LOGLEVEL_INFO, "not a dvd");
        success = false;
    }
    DVDClose (reader);
    if (success) {
        keylist = mKeylist;
    }
    return (success);
}

