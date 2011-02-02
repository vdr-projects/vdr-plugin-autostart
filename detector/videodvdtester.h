/*
 * videodvdtester.h: Detects Video DVDs, using libdvdread.
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef VIDEODVDTESTER_H_
#define VIDEODVDTESTER_H_

#include "mediatester.h"

class cVideoDVDTester : public cMediaTester
{
public:
    cVideoDVDTester(cLogger *l, std::string descr, std::string ext) :
            cMediaTester (l, descr, ext) {};
    bool isMedia(cMediaHandle d, stringList &keylist);
    cMediaTester *create(cLogger *l) const {
        return new cVideoDVDTester(l, mDescription, mExt);
    }
};

#endif /* VIDEODVDTESTER_H_ */
