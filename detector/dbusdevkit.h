/*
 * dbusdevkit.h: Accesses devicekit disks or udisksvia the dbus
 *               interface
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef DBUSDEVKIT_H_
#define DBUSDEVKIT_H_

#include <dbus/dbus.h>
#include <string>
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

    std::string FindDeviceByDeviceFile (const std::string device);
    stringList EnumerateDevices (void);
    // Do automount and return mount path
    std::string AutoMount(const std::string path)
                                throw (cDeviceKitException);
    void UnMount (const std::string &path)
                                throw (cDeviceKitException) {
        CallInterfaceV (path, "FilesystemUnmount");
    }
    std::string GetNativePath (const std::string &path)
                                 throw (cDeviceKitException) {
        const std::string name = "native-path";
        return GetDbusPropertyS (path, name);
    }

    std::string GetType (const std::string &path)
                          throw (cDeviceKitException) {
        const std::string name = "id-type";
        return GetDbusPropertyS (path, name);
    }

    std::string GetDeviceFile (const std::string &path)
                                 throw (cDeviceKitException) {
        const std::string name = "device-file";
        return GetDbusPropertyS (path, name);
    }
    stringList GetMountPaths (const std::string &path)
                                 throw (cDeviceKitException) {
        const std::string name = "DeviceMountPaths";
        return GetDbusPropertyAS (path, name);
    }

    bool IsMounted(const std::string &path)
                     throw (cDeviceKitException){
        const std::string name = "device-is-mounted";
        return GetDbusPropertyB (path, name);
    }

    bool IsOpticalDisk(const std::string &path)
                         throw (cDeviceKitException) {
        const std::string name = "device-is-optical-disc";
        return GetDbusPropertyB (path, name);
    }
    bool IsPartition(const std::string &path)
                       throw (cDeviceKitException) {
        const std::string name = "device-is-partition";
        return GetDbusPropertyB (path, name);
    }
    bool IsMediaAvailable(const std::string &path)
                             throw (cDeviceKitException) {
        const std::string name = "device-is-media-available";
        return GetDbusPropertyB (path, name);
    }
  private:
    DBusConnection *mConnSystem;
    DBusError mErr;
    cLogger *mLogger;

    static const char *DEVICEKIT_DISKS_SERVICE;
    static const char *UDISKS_SERVICE;
    static const char *DBUS_NAME;
    static const char *DEVICEKIT_DISKS_OBJECT;
    static const char *UDISKS_OBJECT;

    const char *mService;
    const char *mObjectPath;

    std::string mDevicekitInterface;

    DBusMessage *CallDbusProperty (const std::string &path,
                                     const std::string &name,
                                     DBusMessageIter *iter)
                                     throw (cDeviceKitException);
     // Property s
    std::string GetDbusPropertyS (const std::string &path,
                                    const std::string &name)
                                    throw (cDeviceKitException);
    // Property as (String Array)
    stringList GetDbusPropertyAS (const std::string &path,
                                     const std::string &name)
                                     throw (cDeviceKitException);
    // Property u
    dbus_int32_t GetDbusPropertyU (const std::string &path,
                                     const std::string &name)
                                     throw (cDeviceKitException);
    // Property b
    bool GetDbusPropertyB (const std::string &path,
                              const std::string &name)
                              throw (cDeviceKitException);
    bool WaitConn (void) throw (cDeviceKitException);


    // Call an interface method which does not return a value
    void CallInterfaceV(const std::string &path,
                           const std::string &name)
                           throw (cDeviceKitException);


};

#endif /* DBUSDEVKIT_H_ */
