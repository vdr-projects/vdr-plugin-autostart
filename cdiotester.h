/*
 * CdioTester.h
 *
 *  Created on: 02.10.2010
 *      Author: uli
 */

#ifndef CDIOTESTER_H_
#define CDIOTESTER_H_

#include "mediatester.h"
#include "extstring.h"

class cCdioTester : public cMediaTester
{
public:
    cCdioTester(cLogger *l, std::string descr, std::string ext) {
        mLogger = l;
        mDescription = descr;
        mExt = ext;
    }
    bool isMedia (cMediaHandle d, cExtStringVector &keylist);
    cMediaTester *create(cLogger *l) const {
        return new cCdioTester(l, mDescription, mExt);
    }
};

#endif /* CDIOTESTER_H_ */
