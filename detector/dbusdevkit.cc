/*
 * dbusdevkit.cc: Accesses devicekit disks or udisksvia the dbus
 *                interface
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "dbusdevkit.h"

using namespace std;

const char *cDbusDevkit::DEVICEKIT_DISKS_SERVICE = "org.freedesktop.DeviceKit.Disks";
const char *cDbusDevkit::DBUS_NAME = "vdr.plugin.mediadetector";
const char *cDbusDevkit::UDISKS_SERVICE = "org.freedesktop.UDisks";
const char *cDbusDevkit::DEVICEKIT_DISKS_OBJECT = "/org/freedesktop/DeviceKit/Disks";
const char *cDbusDevkit::UDISKS_OBJECT = "/org/freedesktop/UDisks";

cDeviceKitException::cDeviceKitException (const char *file, int line,
                                                const std::string errtxt)
{
    char buf[40];
    sprintf(buf, ", %d, ", line);
    std::string fn = file;
    mErrTxt = fn + buf + errtxt;
}

cDbusDevkit::cDbusDevkit(cLogger *logger)
{
    mConnSystem = NULL;
    mLogger = logger;
    // initialize the errors
    dbus_error_init(&mErr);
}

cDbusDevkit::~cDbusDevkit()
{
    if (mConnSystem != NULL) {
        dbus_connection_unref (mConnSystem);
    }
}
bool cDbusDevkit::WaitConn (void) throw (cDeviceKitException)
{
    // connect to the bus and check for errors
    if (mConnSystem != NULL) {
        return true;
    }

    mConnSystem = dbus_bus_get (DBUS_BUS_SYSTEM, &mErr);
    if (dbus_error_is_set(&mErr)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Connection Error (%s)", mErr.message);
        dbus_error_free(&mErr);
    }
    if (mConnSystem  == NULL) {
        return false;
    }

    // Check if DEVICEKIT DISK or UDISK is available
    mService = DEVICEKIT_DISKS_SERVICE;
    mObjectPath = DEVICEKIT_DISKS_OBJECT;

    if (!dbus_bus_start_service_by_name(mConnSystem, mService, 0,
                                        NULL, &mErr)) {
        mLogger->logmsg(LOGLEVEL_INFO, "Probe DeviceKit Disks (%s)", mErr.message);
        dbus_error_free(&mErr);
        mService = UDISKS_SERVICE;
        mObjectPath = UDISKS_OBJECT;
        if (!dbus_bus_start_service_by_name(mConnSystem, mService, 0,
                                        NULL, &mErr)) {
            mLogger->logmsg(LOGLEVEL_ERROR, "No connection (%s)", mErr.message);
            dbus_error_free(&mErr);
            return false;
        }
        mLogger->logmsg(LOGLEVEL_WARNING, "UDisks found");
    }
    else {
        mLogger->logmsg(LOGLEVEL_WARNING, "Devicekit Disks found");
    }

    mDevicekitInterface = mService;
    mDevicekitInterface += ".Device";


    string rule ="type='signal'"
                 ",interface='";
    rule = rule + mService + "'";

    // add a filter for the messages we want to see
    dbus_bus_add_match (mConnSystem, rule.c_str(), &mErr); // see signals from the given interface
    if (dbus_error_is_set(&mErr)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Match Error (%s)", mErr.message);
        dbus_error_free(&mErr);
        return false;
    }

    return true;
}

bool cDbusDevkit::WaitDevkit(int timeout, string &retpath, DEVICE_SIGNAL &signal)
{
    DBusMessage *devkitmsg;
    bool sigrcv = false;
    char *path = NULL;

    while (!WaitConn()) {
        sleep(1);
    }

    do {
        dbus_connection_read_write(mConnSystem, timeout);
        devkitmsg = dbus_connection_pop_message(mConnSystem);
        if (devkitmsg != NULL) {
            mLogger->logmsg(LOGLEVEL_INFO, "Message received %s\n Member %s",
                    dbus_message_get_interface(devkitmsg),
                    dbus_message_get_member(devkitmsg));
            signal = Unkown;
            // check if the message is a signal from the correct interface and with the correct name
            if (dbus_message_is_signal(devkitmsg, mService, "DeviceAdded")) {
                signal = DeviceAdded;
            }
            else if (dbus_message_is_signal(devkitmsg, mService, "DeviceRemoved")) {
                signal = DeviceRemoved;
            }
            else if (dbus_message_is_signal(devkitmsg, mService, "DeviceChanged")) {
                signal = DeviceChanged;
            }
            if (signal != Unkown) {
                // read the parameters
                DBusMessageIter iter;
                path = NULL;
                if (!dbus_message_iter_init(devkitmsg, &iter)) {
                    mLogger->logmsg(LOGLEVEL_WARNING, "Signal has no argument!");
                } else {
                    int type = dbus_message_iter_get_arg_type(&iter);
                    if (type != DBUS_TYPE_OBJECT_PATH) {
                        mLogger->logmsg(LOGLEVEL_WARNING, "Argument is not object path");
                    } else {
                        dbus_message_iter_get_basic(&iter, &path);
                    }
                }

                if (path != NULL) {
                    // read the parameters
                    mLogger->logmsg(LOGLEVEL_INFO, "DeviceChange Signal for %s", path);
                    retpath = path;
                    sigrcv = true;
                }
            }
            dbus_message_unref(devkitmsg);
        }
        else {
            retpath = "";
            return false;
        }
    } while (!sigrcv);
    return true;
}

string cDbusDevkit::FindDeviceByDeviceFile (const string device)
{
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;
    DBusMessageIter iter;
    char *val;
    string retval;
    const char *dev = device.c_str();

    WaitConn();

    getmsg = dbus_message_new_method_call(mService,   // target for the method call
                                       mObjectPath,       // object to call on
                                       mService,          // interface to call on
                                       "FindDeviceByDeviceFile");   // method name
    if (getmsg == NULL) {
        DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
    }

    try {
        dbus_message_append_args(getmsg, DBUS_TYPE_STRING, &dev,
                                     DBUS_TYPE_INVALID);

        // send message and get a handle for a reply
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                              -1, &mErr);
        if (dbus_error_is_set(&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed";
            errmsg += mErr.message;
            dbus_error_free(&mErr);
            DEVKITEXCEPTION(errmsg);
        }

        // read the parameters
        if (!dbus_message_iter_init(msg, &iter)) {
            DEVKITEXCEPTION("Message has no arguments " + device);
        }

        int msgtype = dbus_message_iter_get_arg_type(&iter);
        if (msgtype != DBUS_TYPE_OBJECT_PATH) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Object Path %c!",
                                            msgtype);
            DEVKITEXCEPTION("Argument is not String " + device);
        }
        dbus_message_iter_get_basic(&iter, &val);
        retval = val;

        // free message
        dbus_message_unref(getmsg);
        // free reply and close connection
        dbus_message_unref(msg);
    } catch (cDeviceKitException &e) {
        if (getmsg != NULL) {
            dbus_message_unref(getmsg);
        }
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        throw;
    }
    return retval;
}

stringList cDbusDevkit::EnumerateDevices (void)
{
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;
    DBusMessageIter iter;
    DBusMessageIter subiter;
    stringList retval;
    char *val;

    WaitConn();

    getmsg = dbus_message_new_method_call(mService,   // target for the method call
                                       mObjectPath,       // object to call on
                                       mService,          // interface to call on
                                       "EnumerateDevices");   // method name
    if (getmsg == NULL) {
        DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
    }

    try {
        // send message and get a handle for a reply
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                              -1, &mErr);
        if (dbus_error_is_set(&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed";
            errmsg += mErr.message;
            dbus_error_free(&mErr);
            DEVKITEXCEPTION(errmsg);
        }

        // read the parameters
        if (!dbus_message_iter_init(msg, &iter)) {
            DEVKITEXCEPTION("Message has no arguments!");
        }

        int msgtype = dbus_message_iter_get_arg_type(&iter);
        if (msgtype != DBUS_TYPE_ARRAY) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not array %c!",
                    msgtype);
            DEVKITEXCEPTION("Argument is not a array");
        }

        dbus_message_iter_recurse(&iter, &subiter);
        msgtype = dbus_message_iter_get_arg_type(&subiter);
        if (msgtype != DBUS_TYPE_OBJECT_PATH) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not object path %c!",
                    msgtype);
            DEVKITEXCEPTION("Argument is not object path");
        }

        do {
            dbus_message_iter_get_basic(&subiter, &val);
            retval.push_back(val);
        } while (dbus_message_iter_next(&subiter));
        // free message
        dbus_message_unref(getmsg);
        // free reply and close connection
        dbus_message_unref(msg);
    } catch (cDeviceKitException &e) {
        if (getmsg != NULL) {
            dbus_message_unref(getmsg);
        }
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        throw;
    }
    return retval;
}


DBusMessage *cDbusDevkit::CallDbusProperty (const string &path,
                                                const string &name,
                                                DBusMessageIter *iter)
                                                throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;

    getmsg = dbus_message_new_method_call(mService,   // target for the method call
                                       path.c_str(),                // object to call on
                                       "org.freedesktop.DBus.Properties", // interface to call on
                                       "Get"); // method name
    if (getmsg == NULL) {
        DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
    }

    try {
        const char *dev_inter = mDevicekitInterface.c_str();
        const char *na = name.c_str();

        dbus_message_append_args(getmsg, DBUS_TYPE_STRING, &dev_inter,
                                    DBUS_TYPE_STRING, &na, DBUS_TYPE_INVALID);

        // send message and get a handle for a reply
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                              -1, &mErr);
        if (dbus_error_is_set(&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed";
            errmsg += mErr.message;
            dbus_error_free(&mErr);
            DEVKITEXCEPTION(errmsg);
        }

        // read the parameters
        if (!dbus_message_iter_init(msg, iter)) {

            DEVKITEXCEPTION("Message has no arguments " + name);
        }

        int msgtype = dbus_message_iter_get_arg_type(iter);
        if (msgtype != DBUS_TYPE_VARIANT) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Variant %c!",
                    msgtype);
            DEVKITEXCEPTION("Argument is not Variant " + name);
        }
        // free message
        dbus_message_unref(getmsg);
    } catch (cDeviceKitException &e) {
        if (getmsg != NULL) {
            dbus_message_unref(getmsg);
        }
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        throw;
    }
    return msg;
}

string cDbusDevkit::GetDbusPropertyS (const string &path,
                                          const string &name)
                                          throw (cDeviceKitException)
{
    char *val;
    DBusMessage *msg = NULL;
    DBusMessageIter subiter;
    DBusMessageIter iter;
    string retval;

    msg = CallDbusProperty(path, name, &iter);
    dbus_message_iter_recurse(&iter, &subiter);
    int msgtype = dbus_message_iter_get_arg_type(&subiter);
    if (msgtype != DBUS_TYPE_STRING) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not String %c!", msgtype);
        dbus_message_unref(msg);
        DEVKITEXCEPTION ("Argument is not a String");
    }
    dbus_message_iter_get_basic(&subiter, &val);
    retval = val;
    // free reply and close connection
    dbus_message_unref(msg);
    return retval;
}

stringList cDbusDevkit::GetDbusPropertyAS (const string &path,
                                               const string &name)
                                               throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessageIter iter;
    DBusMessageIter subiter;
    stringList retval;
    string s;
    char *val;

    msg = CallDbusProperty(path, name, &iter);
    try {
        if (!dbus_message_iter_init(msg, &iter)) {
            DEVKITEXCEPTION("Message has no arguments!");
        }

        int msgtype = dbus_message_iter_get_arg_type(&iter);
        if (msgtype != DBUS_TYPE_VARIANT) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Variant %c!",
                    msgtype);
            DEVKITEXCEPTION("Argument is not Variant");
        }
        dbus_message_iter_recurse(&iter, &subiter);
        msgtype = dbus_message_iter_get_arg_type(&subiter);
        if (msgtype != DBUS_TYPE_ARRAY) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Array %c!",
                    msgtype);
            DEVKITEXCEPTION("Argument is not Array");
        }
        dbus_message_iter_recurse(&subiter, &iter);
        msgtype = dbus_message_iter_get_arg_type(&iter);
        if (msgtype != DBUS_TYPE_STRING) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not String %c!",
                    msgtype);
            DEVKITEXCEPTION("Argument is not Array");
        }

        do {
            dbus_message_iter_get_basic(&iter, &val);
            s = val;
            retval.push_back(s);
        } while (dbus_message_iter_next(&iter));
    } catch (cDeviceKitException &e) {
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        throw;
    }
    // free reply and close connection
    dbus_message_unref(msg);
    return retval;
}

dbus_int32_t cDbusDevkit::GetDbusPropertyU (const string &path,
                                               const string &name)
                                               throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    dbus_int32_t *val, retval;
    DBusMessageIter subiter;
    DBusMessageIter iter;

    msg = CallDbusProperty(path, name, &iter);
    dbus_message_iter_recurse(&iter, &subiter);
    int msgtype = dbus_message_iter_get_arg_type(&subiter);
    if (msgtype != DBUS_TYPE_UINT32) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not int %c!", msgtype);
        dbus_message_unref(msg);
        DEVKITEXCEPTION ("Argument is not int");
    }
    dbus_message_iter_get_basic(&subiter, &val);
    retval = *val;
    // free reply and close connection
    dbus_message_unref(msg);
    return retval;
}

bool cDbusDevkit::GetDbusPropertyB (const string &path,
                                       const string &name)
                                       throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    dbus_bool_t retval;
    DBusMessageIter subiter;
    DBusMessageIter iter;

    msg = CallDbusProperty(path, name, &iter);
    dbus_message_iter_recurse(&iter, &subiter);
    int msgtype = dbus_message_iter_get_arg_type(&subiter);
    if (msgtype != DBUS_TYPE_BOOLEAN) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not bool %c!", msgtype);
        dbus_message_unref(msg);
        DEVKITEXCEPTION ("Argument is not bool");
    }
    dbus_message_iter_get_basic(&subiter, &retval);
    // free reply and close connection
    dbus_message_unref(msg);
    return retval;
}

string cDbusDevkit::AutoMount(const string path)
    throw (cDeviceKitException)
{
    DBusMessage *msg, *getmsg;
    const char *fs_type = "";
    const char *opts[] = {NULL};
    char *val;
    int argcnt = 0;
    string retval;

    try {
        getmsg = dbus_message_new_method_call(mService, // target for the method call
                path.c_str(), // object to call on
                mDevicekitInterface.c_str(), // interface to call on
                "FilesystemMount"); // method name
        if (getmsg == NULL) {
            DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
        }

        dbus_message_append_args(getmsg, DBUS_TYPE_STRING, &fs_type,
                DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, opts, &argcnt,
                DBUS_TYPE_INVALID);
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                -1, &mErr);
        if (dbus_error_is_set(&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed";
            errmsg += mErr.message;
            dbus_error_free(&mErr);
            DEVKITEXCEPTION(errmsg);
        }

        DBusMessageIter args;
        // read the parameters
        if (!dbus_message_iter_init(msg, &args)) {
            dbus_message_unref(msg);
            DEVKITEXCEPTION("Message has no arguments!");
        }

        int msgtype = dbus_message_iter_get_arg_type(&args);
        if (msgtype != DBUS_TYPE_STRING) {
            dbus_message_unref(msg);
            mLogger->logmsg(LOGLEVEL_ERROR, "Return value is not String %c!",
                    msgtype);
            DEVKITEXCEPTION("Return value is not String");
        }
        dbus_message_iter_get_basic(&args, &val);
    } catch (cDeviceKitException &e) {
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        throw;
    }

    retval = val;
    dbus_message_unref(msg);
    return retval;
}

void cDbusDevkit::CallInterfaceV(const string &path,
                                       const string &name)
    throw (cDeviceKitException)
{
    DBusMessage *msg, *getmsg;
    char *dbusarr[] = {};

    getmsg = dbus_message_new_method_call(mService,   // target for the method call
                                       path.c_str(),                // object to call on
                                       mDevicekitInterface.c_str(), // interface to call on
                                       name.c_str());              // method name
    if (getmsg == NULL) {
        DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
    }

    dbus_message_append_args (getmsg,
                           DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, dbusarr, 0,
                           DBUS_TYPE_INVALID);
    msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                          -1,  &mErr);
    if (dbus_error_is_set(&mErr)) {
        string errmsg = "dbus_connection_send_with_reply failed";
        errmsg += mErr.message;
        dbus_error_free(&mErr);
        DEVKITEXCEPTION(errmsg);
    }

    dbus_message_unref (msg);
}
