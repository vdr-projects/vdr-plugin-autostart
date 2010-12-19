/*
 * detectortest.c: Testprogramm for the media detection framework.
 *
 * To compile the testprogramm use "make detectortest"
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include <stdbool.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "mediadetector.h"
#include "configfileparser.h"

using namespace std;

void logkeylist(ValueList vl)
{
    cLogger log;
    ValueList::iterator it;
    for (it = vl.begin(); it != vl.end(); it++) {
        log.logmsg(LOGLEVEL_ERROR, "   %s", it->c_str());
    }
}
int main()
{
    cLogger log;
    cMediaDetector detector;
    ValueList vl;
    string descr;
    cMediaHandle ha;

    detector.InitDetector(log, "/tmp/test.conf");
    while (true) {
        vl = detector.Detect(descr, ha);
        log.logmsg(LOGLEVEL_ERROR, "\n%s Keylist : ", descr.c_str());
        logkeylist(vl);
    }
}
