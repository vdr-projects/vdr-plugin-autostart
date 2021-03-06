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

#include "configmenu.h"

static const char *workingmode_entry[] = {
        tr("Automatic"),
        tr("Manual"),
};

const char *cConfigMenu::WORKINGMODE = "Mode";
const char *cConfigMenu::ENABLEMAINMENU = "EnableMainMenu";
int cConfigMenu::mShowMainMenu = true;
int cConfigMenu::mWorkingMode = cMediaDetector::AUTO_START;
cMediaDetectorThread *cConfigMenu::mDetector = NULL;

cConfigMenu::cConfigMenu(cMediaDetectorThread *detector) : cMenuSetupPage()
{
    mDetector = detector;
    SetSection (tr("Autostart"));

    Add(new cMenuEditStraItem(tr("Mode"), &mWorkingMode,
                               (int)cMediaDetector::LAST_MODE, workingmode_entry));
    Add(new cMenuEditBoolItem(tr("Show in main menu"), &mShowMainMenu));
}

const bool cConfigMenu::SetupParse(const char *Name, const char *Value)
{
  // Parse setup parameters and store their values.
  if (strcasecmp(Name, WORKINGMODE) == 0) {
      mWorkingMode = atoi(Value);
      if (mDetector != NULL) {
          mDetector->SetWorkingMode((cMediaDetector::WORKING_MODE)mWorkingMode);
      }
  }
  else if (strcasecmp(Name, ENABLEMAINMENU) == 0) {
      mShowMainMenu = atoi(Value);
  }
  else {
     return false;
  }
  return true;
}

void cConfigMenu::Store(void)
{
    SetupStore(WORKINGMODE, mWorkingMode);
    SetupStore(ENABLEMAINMENU, (int)mShowMainMenu);
    if (mDetector != NULL) {
        mDetector->SetWorkingMode((cMediaDetector::WORKING_MODE)mWorkingMode);
    }
}
