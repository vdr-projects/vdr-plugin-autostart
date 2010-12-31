/*
 * mediadetectorthread.cc: Main thread for media detection.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#include "mediadetectorthread.h"
#include "autostart.h"
#include "autostartservice.h"
#include <vdr/thread.h>

using namespace std;

#ifdef DEBUG
void cMediaDetectorThread::logkeylist(cExtStringVector vl)
{
    cLogger log;
    cExtStringVector::iterator it;
    for (it = vl.begin(); it != vl.end(); it++) {
        mLogger.logmsg(LOGLEVEL_ERROR, "   %s", it->c_str());
    }
}
#endif

void cMediaDetectorThread::Action(void)
{
    cExtStringVector vl;
    cPlugin *p;
    AutoStartService service;
    cMediaHandle mediadescr(&mLogger);
    string des;

    sleep(5);
    while (Running()) {
        vl = mDetector.Detect(des, mediadescr);
        if (!vl.empty()) {
            service.mDescription = des;
            service.mKeyList = vl;
            service.mMediaDescr = mediadescr;
            // First send to service to own plugin
            p = cPluginManager::GetPlugin(mPluginName.c_str());
            if (p == NULL) {
                mLogger.logmsg(LOGLEVEL_ERROR, "Own Plugin %s not found",
                        mPluginName.c_str());
                exit(-1);

            }
            service.mSendToOwn = true;
            p->Service(AUTOSTART_SERVICE_ID, &service);

            // Send to all plugins in case an other plugin is interested.
            service.mSendToOwn = false;
            cPluginManager::CallFirstService(AUTOSTART_SERVICE_ID, &service);
        }
#ifdef DEBUG
        mLogger.logmsg(LOGLEVEL_ERROR, "\n%s Keylist : ", des.c_str());
        logkeylist(vl);
#endif
    }
}
