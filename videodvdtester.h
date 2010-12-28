/*
 * videodvdtester.h
 *
 *  Created on: 02.10.2010
 *      Author: uli
 */

#ifndef VIDEODVDTESTER_H_
#define VIDEODVDTESTER_H_

#include "mediatester.h"

class cVideoDVDTester : public cMediaTester
{
public:
    cVideoDVDTester(cLogger *l, std::string descr, std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
    }
    bool isMedia(cMediaHandle d, cExtStringVector &keylist);
    cMediaTester *create(cLogger *l) const {
        return new cVideoDVDTester(l, mDescription, mExt);
    }
};

#endif /* VIDEODVDTESTER_H_ */
