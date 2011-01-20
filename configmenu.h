/*
 * Autostart PlugIn for VDR
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the configuration menu
 *
 */

#ifndef CONFIGMENU_H_
#define CONFIGMENU_H_

#include <vdr/plugin.h>
#include "detector/mediadetector.h"
#include "autostart.h"
#include "mediadetectorthread.h"

class cConfigMenu: public cMenuSetupPage {
    friend class cPluginAutostart;
private:
    static int mWorkingMode;
    static int mShowMainMenu;
    static const char *WORKINGMODE;
    static const char *ENABLEMAINMENU;
    static cMediaDetectorThread *mDetector;

    cConfigMenu(void) {};

protected:
    virtual void Store(void);

public:

    cConfigMenu(cMediaDetectorThread *detector);
    static const cMediaDetector::WORKING_MODE GetWorkingMode(void) {
        return (cMediaDetector::WORKING_MODE)mWorkingMode;
    }
    static const bool GetShowMainMenu(void) { return mShowMainMenu; }
    static const bool SetupParse(const char *Name, const char *Value);
};

#endif /* CONFIGMENU_H_ */
