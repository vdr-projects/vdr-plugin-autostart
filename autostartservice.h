/*
 * autostartservice.h: Definitions for autostart service
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#ifndef AUTOSTARTSERVICE_H_
#define AUTOSTARTSERVICE_H_

#include <string>
#include "detector/mediadetector.h"

#define AUTOSTART_SERVICE_ID "AutostartPlugin-V0.0.1"

typedef struct _autostart_service {
    std::string mDescription;
    std::string mMountPath;
    stringList mKeyList;
    cMediaHandle mMediaDescr;
    bool mSendToOwn;
} AutoStartService;

#endif /* AUTOSTARTSERVICE_H_ */
