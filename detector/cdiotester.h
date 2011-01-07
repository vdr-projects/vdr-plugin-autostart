/*
 * cdiotester.h: Detects audio CDs, using libcdio.
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef CDIOTESTER_H_
#define CDIOTESTER_H_

#include "mediatester.h"
#include "stringtools.h"

class cCdioTester : public cMediaTester
{
public:
    cCdioTester(cLogger *l, const std::string descr, const std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
    }
    bool isMedia (cMediaHandle d, stringList &keylist);
    cMediaTester *create(cLogger *l) const {
        return new cCdioTester(l, mDescription, mExt);
    }
};

#endif /* CDIOTESTER_H_ */
