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

class cConfigMenu: public cMenuSetupPage {
public:
    cConfigMenu(void);
    static const cMediaDetector::WORKING_MODE GetWorkingMode(void) {
        return (cMediaDetector::WORKING_MODE)mWorkingMode;
    }
    static const bool GetShowMainMenu(void) { return mShowMainMenu; }
    static const bool SetupParse(const char *Name, const char *Value);

    static const char *WORKINGMODE;
    static const char *ENABLEMAINMENU;
private:
    static int mWorkingMode;
    static int mShowMainMenu;

protected:
    virtual void Store(void);
};

#endif /* CONFIGMENU_H_ */
