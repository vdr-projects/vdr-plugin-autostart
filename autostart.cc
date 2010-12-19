/*
 * autostart.c: A plugin for the Video Disk Recorder
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

static const char *MAINMENUENTRY  = trNOOP("Autostart");

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
    dsyslog("Initialize");
    mDetector.SetDescription ("MediaDetector");
    return mDetector.InitDetector(GetConfigFile(), Name());
}

bool cPluginAutostart::Start(void)
{
    dsyslog("Start");
    mDetector.Start();
    return true;
}

void cPluginAutostart::Stop(void)
{
    dsyslog("Stop");
    mDetector.Stop();
}

cString cPluginAutostart::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

cOsdObject *cPluginAutostart::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginAutostart::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginAutostart::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginAutostart::Service(const char *Id, void *Data)
{
    dsyslog("Service %s received\n", Id);
    cMutexLock MutexLock(&mAutostartMutex);
    if (strcmp(Id, autostart_service_id) == 0) {
        if (Data) {
            AutoStartService *se = (AutoStartService *)Data;
            if (se->mSendToOwn) {
                dsyslog("Descr %s", se->mDescription.c_str());
                ProcessService(*se);
                return true;
            }
        }
    }
    return false;
}

void cPluginAutostart::ProcessService(const AutoStartService as)
{
    ValueList vl = as.mKeyList;
    ValueList::iterator it;
    cPlugin *p;

    for (it = vl.begin(); it != vl.end(); it++) {
        string key = *it;
        switch (key[0]) {
        case '@':      // Plugin
            key.erase(0,1);
            p = cPluginManager::GetPlugin(key.c_str());
            if (p == NULL) {
                esyslog("Plugin %s not available", key.c_str());
                return;
            }
            p->MainMenuAction();
            break;
        case '#':       // Script
            key.erase(0,1);
            break;
        default:        // Key
            eKeys keycode = cKey::FromString(key.c_str());
            if (keycode == kNone) {
                esyslog("Unkown key %s", key.c_str());
                return;
            }
            cRemote::Put (keycode);
            break;
        }
    }
}

const char **cPluginAutostart::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginAutostart::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}

const char *cPluginAutostart::MainMenuEntry(void)
{
    return tr(MAINMENUENTRY);
}

VDRPLUGINCREATOR(cPluginAutostart); // Don't touch this!
