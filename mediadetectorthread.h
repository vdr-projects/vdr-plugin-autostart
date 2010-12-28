/*
 * mediadetectorthread.h: Main thread for media detection.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#ifndef MEDIADETECTORTHREAD_H_
#define MEDIADETECTORTHREAD_H_

#include <string>
#include <vdr/thread.h>
#include "mediadetector.h"

class cMediaDetectorThread : public cThread {
private:
    // cLogger mLogger; // Log to stdout
    cVdrLogger mLogger; // Use VDR Logging
    cMediaDetector mDetector;
    std::string mPluginName;
#ifdef DEBUG
    void logkeylist(cExtStringVector vl);
#endif

public:
    cMediaDetectorThread() : mDetector(&mLogger) {};
    bool InitDetector(const std::string inifile, const std::string pluginname) {
        mPluginName = pluginname;
        return mDetector.InitDetector(&mLogger, inifile);
    }
    virtual void Action(void);
    void Stop(void) {
        mDetector.Stop();
        Cancel(5);
    }
};

#endif /* MEDIADETECTORTHREAD_H_ */
