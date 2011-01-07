/*
 * detectortest.c: Testprogramm for the media detection framework.
 *
 * To compile the testprogramm use "make test"
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

void logkeylist(stringList vl)
{
    cLogger log;
    stringList::iterator it;
    for (it = vl.begin(); it != vl.end(); it++) {
        log.logmsg(LOGLEVEL_ERROR, "   %s", it->c_str());
    }
}
int main(int argc, const char* argv[])
{
    cLogger log;
    cMediaDetector detector(&log);
    stringList vl;
    string descr;
    cMediaHandle ha;
    int i;

    detector.InitDetector(&log, "/tmp/test.conf");
    for (i=0; i < 3; i++) {
        vl = detector.Detect(descr, ha);
        log.logmsg(LOGLEVEL_ERROR, "\n%s Keylist : ", descr.c_str());
        logkeylist(vl);
    }
}
