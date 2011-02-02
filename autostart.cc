/*
 * autostart.cc: A plugin for the Video Disk Recorder
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include <getopt.h>
#include <stdlib.h>
#include <vdr/keys.h>
#include <vdr/remote.h>
#include "autostart.h"

using namespace std;

static const char *MAINMENUENTRY  = tr("Autostart");

string cPluginAutostart::mCfgDir = "autostart";
string cPluginAutostart::mCfgFile = "autostart.conf";

cPluginAutostart::cPluginAutostart(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginAutostart::~cPluginAutostart()
{
  // Clean up after yourself!
}

const char *cPluginAutostart::CommandLineHelp(void)
{
    return "-c  --configdir <dir>     Directory for config files : autostart\n";
}

bool cPluginAutostart::ProcessArgs(int argc, char *argv[])
{
    static struct option long_options[] =
    {
        { "configdir",      required_argument, NULL, 'c' },
        { NULL }
    };
    int c, option_index = 0;

    while ((c = getopt_long(argc, argv, "c:",
                            long_options, &option_index)) != -1) {
        switch (c) {
        case 'c':
            mCfgDir.assign(optarg);
            break;
        default:
            esyslog("Autostart unknown option %c", c);
            return false;
        }
    }

    return true;
}

bool cPluginAutostart::Initialize(void)
{
    mDetector.SetDescription ("MediaDetector");
    return mDetector.InitDetector(GetConfigFile(), Name());
}

bool cPluginAutostart::Start(void)
{
    mDetector.Start();
    mDetector.SetWorkingMode(cConfigMenu::GetWorkingMode());
    return true;
}

void cPluginAutostart::Stop(void)
{
    mDetector.Stop();
}

cString cPluginAutostart::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

const char *cPluginAutostart::MainMenuEntry(void)
{
    if (cConfigMenu::GetShowMainMenu()) {
        return tr(MAINMENUENTRY);
    }
    return NULL;
}

cOsdObject *cPluginAutostart::MainMenuAction(void)
{
  mDetector.StartManualScan();
  return NULL;
}

cMenuSetupPage *cPluginAutostart::SetupMenu(void)
{
  // Return the setup menu
  return new cConfigMenu(&mDetector);
}

bool cPluginAutostart::SetupParse(const char *Name, const char *Value)
{
  // Parse setup parameters and store their values.
  bool ret = cConfigMenu::SetupParse(Name, Value);
  return ret;
}

bool cPluginAutostart::Service(const char *Id, void *Data)
{
    if (Data == NULL) {
        return false;
    }
    dsyslog("Service %s received\n", Id);
    cMutexLock MutexLock(&mAutostartMutex);
    if (strcmp(Id, AUTOSTART_SERVICE_ID) == 0) {
        AutoStartService *se = (AutoStartService *)Data;
        if (se->mSendToOwn) {
            dsyslog("Descr %s", se->mDescription.c_str());
            mSender.Run(*se);
            return true;
        }
    }
    return false;
}

void cSenderThread::Action(void)
{
    stringList vl = as.mKeyList;
    stringList::iterator it;
    cPlugin *p;

    for (it = vl.begin(); it != vl.end(); it++) {
        string key = *it;
        dsyslog("Send key %s", key.c_str());
        switch (key[0]) {
        case '@':      // Plugin
            key.erase(0,1);
            p = cPluginManager::GetPlugin(key.c_str());
            if (p == NULL) {
                esyslog("Plugin %s not available", key.c_str());
                return;
            }
            cRemote::CallPlugin(key.c_str());
            usleep(500);
            break;
/*        case '#':       // Script
            key.erase(0,1);
            break;*/
        default:        // Key
            eKeys keycode = cKey::FromString(key.c_str());
            if (keycode == kNone) {
                esyslog("Unkown key %s", key.c_str());
                return;
            }

            cRemote::Put (keycode);

            usleep(500);
            break;
        }
    }
}

const char **cPluginAutostart::SVDRPHelpPages(void)
{
  static const char *HelpPages[] = {
            "DETECT:  Detect and play\n",
            NULL
    };
    return HelpPages;
}

cString cPluginAutostart::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
    if (strcasecmp(Command, "DETECT") == 0) {
        cRemote::CallPlugin(Name());
        return "OK";
    }
    return NULL;
}

VDRPLUGINCREATOR(cPluginAutostart); // Don't touch this!
