/*
 * dbusdevkit.h: Accesses devicekit disks or udisksvia the dbus
 *               interface
 *
 *
 * Copyright (C) 2010-2018 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef DBUSDEVKIT_H_
#define DBUSDEVKIT_H_

#include <dbus/dbus.h>
#include <string>
#include <string.h>
#include <list>
#include <exception>
#include <stdio.h>
#include "logger.h"
#include "stdtypes.h"

#define DEVKITEXCEPTION(t) throw cDeviceKitException(__FILE__, __LINE__,(t))

class cDeviceKitException : private std::exception
{
private:
    std::string mErrTxt;

public:
    cDeviceKitException (const char *errtxt) : mErrTxt(errtxt) {};
    cDeviceKitException (const std::string errtxt) : mErrTxt(errtxt) {};
    cDeviceKitException (const char *file, int line, const std::string errtxt);

    virtual ~cDeviceKitException () throw () {};
    virtual const char *what(void) const throw () {
        return (mErrTxt.c_str());
    }
};

class cDbusDevkit {
public:
    typedef enum {
        DeviceAdded,
        DeviceRemoved,
        DeviceChanged,
        Unkown
    } DEVICE_SIGNAL;

    cDbusDevkit(cLogger *logger);
    ~cDbusDevkit();
    bool WaitDevkit(int timeout, std::string &retpath, DEVICE_SIGNAL &signal);

    std::string FindDeviceByDeviceFile (const std::string device)
                                                throw (cDeviceKitException);
    stringList EnumerateDevices (void) throw (cDeviceKitException);
    // Do automount and return mount path
    std::string AutoMount(const std::string path) throw (cDeviceKitException);
    void UnMount (const std::string &path) throw (cDeviceKitException);
    std::string GetNativePath (const std::string &path)
                                   throw (cDeviceKitException) ;

    std::string GetType (const std::string &path) throw (cDeviceKitException);
    std::string GetDeviceFile (const std::string &path)
                                           throw (cDeviceKitException);
    stringList GetDeviceFileById (const std::string &path)
                                            throw (cDeviceKitException);
    stringList GetDeviceFileByPath (const std::string &path)
                                             throw (cDeviceKitException);
    stringList GetMountPaths (const std::string &path) throw (cDeviceKitException);
    bool IsMounted(const std::string &path) throw (cDeviceKitException);
    bool IsOpticalDisk(const std::string &path) throw (cDeviceKitException);
    bool IsPartition(const std::string &path) throw (cDeviceKitException);
    bool IsMediaAvailable(const std::string &path) throw (cDeviceKitException);
  private:
    DBusConnection *mConnSystem;
    DBusError mErr;
    cLogger *mLogger;
    bool mUDisk2;

    static const char *DBUS_NAME;
    static const std::string DEVICEKIT_DISKS_SERVICE;
    static const std::string DEVICEKIT_DISKS_OBJECT;

    static const std::string UDISKS_SERVICE;
    static const std::string UDISKS_OBJECT;
    static const std::string UDISKS_INTERFACE;

    static const std::string UDISKS_SERVICE2;
    static const std::string UDISKS_OBJECT2;
    static const std::string UDISKS_OBJECT2_DEV;


    std::string mService;
    std::string mObjectPath;

    std::string GetString(DBusMessageIter &subiter)
                                        throw (cDeviceKitException);
    DBusMessage *CallDbusProperty (const std::string &path,
                                     const std::string &name,
                                     DBusMessageIter *iter,
                                     const std::string &udisk_interface)
                                     throw (cDeviceKitException);

    // Property string (or array of byte)
    std::string GetDbusPropertyS (const std::string &path,
                                    const std::string &name,
                                    const std::string &udisk_interface)
                                    throw (cDeviceKitException);
    // Property as (String Array)
    stringList GetDbusPropertyAS (const std::string &path,
                                     const std::string &name,
                                     const std::string &udisk_interface)
                                     throw (cDeviceKitException);
    // Property integer
    dbus_int32_t GetDbusPropertyU (const std::string &path,
                                     const std::string &name,
                                     const std::string &udisk_interface,
                                     int defaultval = -1)
                                     throw (cDeviceKitException);
    // Property boolean
    bool GetDbusPropertyB (const std::string &path,
                              const std::string &name,
                              const std::string &udisk_interface,
                              bool defaultval = false)
                              throw (cDeviceKitException);
    bool WaitConn (void) throw (cDeviceKitException);


    // Call an interface method which does not return a value
    void CallInterfaceV(const std::string &path,
                        const std::string &name,
                        const std::string &interface)
                           throw (cDeviceKitException);

    // Start a dbus service
    bool StartService(const std::string &name);

    // Udisks2 stuff
    // Optimistic ;-) xml parser
    char *getXML(char **val, const char *tag);

    stringList EnumerateDevices2 (void) throw (cDeviceKitException);
};

#endif /* DBUSDEVKIT_H_ */
