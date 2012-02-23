/*
 * autostart.h: A plugin for the Video Disk Recorder
 *
 * Copyright (C) 2010-2012 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

#ifndef AUTOSTART_H_
#define AUTOSTART_H_

#include <vdr/plugin.h>

#include "mediadetectorthread.h"
#include "configmenu.h"
#include "autostartservice.h"

static const char *VERSION        = "0.9.4";
static const char *DESCRIPTION    = tr("Start a plugin automatically");

class cSenderThread : public cThread {
private:
    AutoStartService as;
public:
    void Run(const AutoStartService s) {
        while (Active()) {
            sleep(1);
        }
        as = s;
        Start();
    }
    virtual void Action(void);
};

class cPluginAutostart: public cPlugin {
private:
    static std::string mCfgDir;
    static std::string mCfgFile;
    cMutex mAutostartMutex;
    AutoStartService mAutoStartService;
    cMediaDetectorThread mDetector;
    cSenderThread mSender;

    static const std::string GetConfigDir(void) {
        const std::string cfdir = cPlugin::ConfigDirectory();
        return cfdir + "/" + mCfgDir + "/";
    }
    static const std::string GetConfigFile(void) {
        const std::string cf = GetConfigDir() + mCfgFile;
        return cf;
    }
    void ProcessService(const AutoStartService as);

public:
    cPluginAutostart(void);
    virtual ~cPluginAutostart();
    virtual const char *Version(void)
    {
        return VERSION;
    }
    virtual const char *Description(void)
    {
        return DESCRIPTION;
    }
    virtual const char *CommandLineHelp(void);
    virtual bool ProcessArgs(int argc, char *argv[]);
    virtual bool Initialize(void);
    virtual bool Start(void);
    virtual void Stop(void);
    virtual void Housekeeping(void) {};
    virtual void MainThreadHook(void) {};
    virtual cString Active(void);
    virtual time_t WakeupTime(void) {return 0;};
    virtual const char *MainMenuEntry(void);
    virtual cOsdObject *MainMenuAction(void);
    virtual cMenuSetupPage *SetupMenu(void);
    virtual bool SetupParse(const char *Name, const char *Value);
    virtual bool Service(const char *Id, void *Data = NULL);
    virtual const char **SVDRPHelpPages(void);
    virtual cString SVDRPCommand(const char *Command, const char *Option,
            int &ReplyCode);
};

#endif /* AUTOSTART_H_ */
