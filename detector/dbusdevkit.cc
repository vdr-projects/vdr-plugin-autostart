/*
 * dbusdevkit.cc: Accesses devicekit disks or udisksvia the dbus
 *                interface
 *
 *
 * Copyright (C) 2010-2018 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "dbusdevkit.h"

using namespace std;

const char *cDbusDevkit::DBUS_NAME = "vdr.plugin.mediadetector";

/* Obsolete devicekit */
const string cDbusDevkit::DEVICEKIT_DISKS_SERVICE = "org.freedesktop.DeviceKit.Disks";
const string cDbusDevkit::DEVICEKIT_DISKS_OBJECT = "/org/freedesktop/DeviceKit/Disks";

/* UDisks 2 */
const string cDbusDevkit::UDISKS_SERVICE2 = "org.freedesktop.UDisks2";
const string cDbusDevkit::UDISKS_OBJECT2 = "/org/freedesktop/UDisks2";
const string cDbusDevkit::UDISKS_OBJECT2_DEV = "/org/freedesktop/UDisks2/block_devices";
/* UDisks */
const string cDbusDevkit::UDISKS_SERVICE = "org.freedesktop.UDisks";
const string cDbusDevkit::UDISKS_OBJECT = "/org/freedesktop/UDisks";
const string cDbusDevkit::UDISKS_INTERFACE = "Device";

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
    mObjectPath.clear();
    mService.clear();
    mUDisk2 = false;
    // initialize the errors
    dbus_error_init(&mErr);
}

cDbusDevkit::~cDbusDevkit()
{
    if (mConnSystem != NULL) {
        dbus_connection_unref (mConnSystem);
    }
}

bool cDbusDevkit::StartService(const string &name)
{
    if (dbus_bus_start_service_by_name(mConnSystem, name.c_str(), 0,
                                       NULL, &mErr)) {
        return true;
    }
    mLogger->logmsg(LOGLEVEL_INFO, "Start service %s (%s)", name.c_str(), mErr.message);
    dbus_error_free(&mErr);
    return false;
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
        mConnSystem = NULL;
        return false;
    }
    if (mConnSystem == NULL) {
        return false;
    }

    // Check if DEVICEKIT DISK2, DEVICEKIT DISK or UDISK is available

    if (StartService(UDISKS_SERVICE2)) {
        mLogger->logmsg(LOGLEVEL_INFO, "Udisks2 found");
        mService = UDISKS_SERVICE2;
        mObjectPath = UDISKS_OBJECT2;
        mUDisk2 = true;
    }
    else if (StartService(UDISKS_SERVICE)) {
        mLogger->logmsg(LOGLEVEL_INFO, "Udisks found");
        mService = UDISKS_SERVICE;
        mObjectPath = UDISKS_OBJECT;
    }
    else if (StartService(DEVICEKIT_DISKS_SERVICE)) {
        mLogger->logmsg(LOGLEVEL_INFO, "Obsolete Devicekit found");
        dbus_error_free(&mErr);
        mService = DEVICEKIT_DISKS_SERVICE;
        mObjectPath = DEVICEKIT_DISKS_OBJECT;
    }
    else {
        mLogger->logmsg(LOGLEVEL_WARNING, "No Devicekit/Udisk Disks found");
        mService.clear();
        mObjectPath.clear();
        return false;
    }

    string rule ="type='signal',interface='";
    if (mUDisk2) {
        rule = rule + "org.freedesktop.DBus.Properties'";
    }
    else {
        rule = rule + mService + "'";
    }

    // add a filter for the messages we want to see
    dbus_bus_add_match (mConnSystem, rule.c_str(), &mErr); // see signals from the given interface
    if (dbus_error_is_set(&mErr)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Match Error (%s)", mErr.message);
        dbus_error_free(&mErr);
        return false;
    }

    return true;
}

/*
 * Wait for the device kit
 * timout : timeout in miliseconds
 */
bool cDbusDevkit::WaitDevkit(int timeout, string &retpath, DEVICE_SIGNAL &signal)
{
    DBusMessage *devkitmsg;
    bool sigrcv = false;
    int conntimeout = 5;
    const char *service;

    while ((!WaitConn()) && (conntimeout > 0)) {
        sleep(1);
        conntimeout--;
    }
    if (conntimeout == 0) {
        mLogger->logmsg(LOGLEVEL_ERROR, "No response from dbus");
        return false;
    }
    if (mUDisk2) {
        service = (const char *)"org.freedesktop.DBus.Properties";
    }
    else {
        service = mService.c_str();
    }
    do {
        dbus_connection_read_write(mConnSystem, timeout);
        devkitmsg = dbus_connection_pop_message(mConnSystem);
        if (devkitmsg != NULL) {
            string path = dbus_message_get_path(devkitmsg);
            /* mLogger->logmsg(LOGLEVEL_INFO, "Message received %s Member %s Path %s",
                    dbus_message_get_interface(devkitmsg),
                    dbus_message_get_member(devkitmsg),
                    path.c_str()); */
            signal = Unkown;
            // check if the message is a signal from the correct interface and with the correct name
            if (dbus_message_is_signal(devkitmsg, service, "DeviceAdded")) {
                signal = DeviceAdded;
            }
            else if (dbus_message_is_signal(devkitmsg, service, "DeviceRemoved")) {
                signal = DeviceRemoved;
            }
            else if (dbus_message_is_signal(devkitmsg, service, "DeviceChanged")) {
                signal = DeviceChanged;
            }
            else if (dbus_message_is_signal(devkitmsg, service, "PropertiesChanged")) {
               if (path.find("/org/freedesktop/UDisks2/block_devices") != string::npos)
               {
                   signal = DeviceChanged;
               }
            }
            if (signal != Unkown) {
                // read the parameters
                mLogger->logmsg(LOGLEVEL_INFO, "DeviceChange Signal for %s", path.c_str());
                retpath = path;
                sigrcv = true;
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

string cDbusDevkit::FindDeviceByDeviceFile (const string device) throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;
    DBusMessageIter iter;
    char *val;
    string retval;
    const char *dev = device.c_str();

    if (!WaitConn()) {
        DEVKITEXCEPTION("No udisk found");
    }

    getmsg = dbus_message_new_method_call(mService.c_str(),   // target for the method call
                                       mObjectPath.c_str(),       // object to call on
                                       mService.c_str(),          // interface to call on
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

/*
 * Optimistic XML "parser"
 */
char *cDbusDevkit::getXML(char **val, const char *tag)
{
    char *start = strstr(*val, tag);
    char *end;
    if (start == NULL){
        return NULL;
    }
    start = strchr(start, '"');
    start++;
    end = strchr(start, '"');
    if (end == NULL) {
        return NULL;
    }
    *end = '\0';
    end++;
    *val = end;
    return (start);
}

stringList cDbusDevkit::EnumerateDevices2 (void) throw (cDeviceKitException)
{
    stringList retval;
    DBusMessage *getmsg = NULL;
    DBusMessage *msg = NULL;
    DBusMessageIter iter;
    char *val;

    getmsg = dbus_message_new_method_call( "org.freedesktop.UDisks2", //mService,   // busname target for the method call
                                            UDISKS_OBJECT2_DEV.c_str(),  // org/freedesktop/UDisks2/block_devices", //mObjectPath,       // object to call on
                                            "org.freedesktop.DBus.Introspectable", //                               mDevicekitInterface.c_str(),    // interface to call on      // interface to call on
                                            "Introspect");   // method name
    try {
           if (getmsg == NULL) {
               DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
           }
           // send message and get a handle for a reply
           msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                                 -1, &mErr);
           if (dbus_error_is_set(&mErr)) {
               string errmsg = "dbus_connection_send_with_reply failed ";
               errmsg += mErr.message;
               dbus_error_free(&mErr);
               DEVKITEXCEPTION(errmsg);
           }

           // read the parameters
           if (!dbus_message_iter_init(msg, &iter)) {
               DEVKITEXCEPTION("Message has no arguments!");
           }
           int msgtype = dbus_message_iter_get_arg_type(&iter);
           if (msgtype != DBUS_TYPE_STRING) {

               mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not a string >%c<!",
                       msgtype);
               DEVKITEXCEPTION("Argument is not a string");
           }
           dbus_message_iter_get_basic(&iter, &val);
#ifdef DEBUG
           mLogger->logmsg(LOGLEVEL_INFO, "Argument %s", val);
#endif
           char *token;
           while ((token = getXML(&val, (const char*)"node name")) != NULL) {
#ifdef DEBUG
           mLogger->logmsg(LOGLEVEL_INFO, "Device %s", token);
#endif
               string s = UDISKS_OBJECT2_DEV;
               s = s + "/";
               s = s + token;
               retval.push_back(s);
           }

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

stringList cDbusDevkit::EnumerateDevices (void) throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;
    DBusMessageIter iter;
    DBusMessageIter subiter;
    stringList retval;
    char *val;

    if (!WaitConn()) {
        DEVKITEXCEPTION("No udisk found");
    }

    if (mUDisk2) {
        return EnumerateDevices2();
    }
    getmsg = dbus_message_new_method_call(mService.c_str(),   // target for the method call
                                       mObjectPath.c_str(),       // object to call on
                                       mService.c_str(),          // interface to call on
                                       "EnumerateDevices");   // method name

    try {
        if (getmsg == NULL) {
            DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
        }

        // send message and get a handle for a reply
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                              -1, &mErr);
        if (dbus_error_is_set(&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed ";
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

            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not array >%c<! ",
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
                                                DBusMessageIter *iter,
                                                const string &udisk_interface)
                                                throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;

    getmsg = dbus_message_new_method_call(mService.c_str(),   // org.freedesktop.UDisks2 target for the method call
                                       path.c_str(),                // object to call on
                                       "org.freedesktop.DBus.Properties", // interface to call on
                                       "Get"); // method name
    if (dbus_error_is_set(&mErr)) {
        string errmsg = "dbus_message_new_method_call failed";
        errmsg += mErr.message;
        dbus_error_free(&mErr);
        DEVKITEXCEPTION(errmsg);
    }
    if (getmsg == NULL) {
        DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
    }

    try {
        string interface = mService + "." + udisk_interface;
        const char *dev_inter = interface.c_str();
        const char *na = name.c_str();

        dbus_message_append_args(getmsg, DBUS_TYPE_STRING, &dev_inter,
                                    DBUS_TYPE_STRING, &na, DBUS_TYPE_INVALID);

        // send message and get a handle for a reply
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                              -1, &mErr);
        if (dbus_error_is_set (&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed ";
            errmsg += mErr.message;
            errmsg += " Name " + name;
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

/*
 * Get string from an iterator of types String, Object Path or Byte Array
 */
string cDbusDevkit::GetString(DBusMessageIter &iter) throw (cDeviceKitException) {
   DBusMessageIter subiter;
   string retval = "";
   char *val;
   dbus_int32_t bval = 0;

   int msgtype = dbus_message_iter_get_arg_type(&iter);
   if ((msgtype == DBUS_TYPE_STRING) || (msgtype == DBUS_TYPE_OBJECT_PATH)) {
       dbus_message_iter_get_basic(&iter, &val);
       retval = val;
   }
   else if (msgtype == DBUS_TYPE_ARRAY) { // Decode byte array
       dbus_message_iter_recurse(&iter, &subiter);
       msgtype = dbus_message_iter_get_arg_type(&subiter);
       if (msgtype == DBUS_TYPE_INVALID) {
           return "";
       }
       if (msgtype == DBUS_TYPE_ARRAY) {
           return GetString(subiter);
       }
       if (msgtype != DBUS_TYPE_BYTE) {
           string err = "Argument is not Byte String >";
           if (msgtype == DBUS_TYPE_INVALID) {
               err += "INVALID";
           }
           else {
               err += msgtype;
           }
           err += "<";
           DEVKITEXCEPTION(err);
       }
       do {
           dbus_message_iter_get_basic(&subiter, &bval);
           if (bval != 0) {
               retval.push_back((char)bval);
           }
       } while (dbus_message_iter_next(&subiter) && ((bval != 0)));
   }
   else {
       string err = "Argument is not a String >";
       if (msgtype == DBUS_TYPE_INVALID) {
           err += "INVALID";
       }
       else {
           err += msgtype;
       }
       err += "<";
       DEVKITEXCEPTION (err);
   }

   return retval;
}

/*
 * Get Property as string
 */
string cDbusDevkit::GetDbusPropertyS (const string &path,
                                          const string &name,
                                          const string &udisk_interface)
                                          throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessageIter subiter;
    DBusMessageIter iter;
    string retval;

    try {
        msg = CallDbusProperty(path, name, &iter, udisk_interface);
    } catch (cDeviceKitException &e) { // Ignore "No such interface"
        return "";
    }
    dbus_message_iter_recurse(&iter, &subiter);

    try {
        retval = GetString(subiter);
    } catch (cDeviceKitException &e) {
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        mLogger->logmsg(LOGLEVEL_ERROR, "%s %s",
                        e.what(), name.c_str());
        throw;
    }
    // free reply and close connection
    dbus_message_unref(msg);
    return retval;
}

/*
 * Get property as array of string
 */
stringList cDbusDevkit::GetDbusPropertyAS (const string &path,
                                               const string &name,
                                               const string &udisk_interface)
                                               throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    DBusMessageIter iter;
    DBusMessageIter subiter;
    stringList retval;
    string s;

    try {
        msg = CallDbusProperty(path, name, &iter, udisk_interface);
    } catch (cDeviceKitException &e) { // Ignore "No such interface"
        return retval;
    }
    try {
        int msgtype = dbus_message_iter_get_arg_type(&iter);
        if (msgtype != DBUS_TYPE_VARIANT) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Variant %c! %s",
                    msgtype, name.c_str());
            DEVKITEXCEPTION("Argument is not Variant");
        }
        dbus_message_iter_recurse(&iter, &subiter);
        msgtype = dbus_message_iter_get_arg_type(&subiter);
        if (msgtype != DBUS_TYPE_ARRAY) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Array %c! %s",
                    msgtype, name.c_str());
            DEVKITEXCEPTION("Argument is not Array");
        }

        dbus_message_iter_recurse(&subiter, &iter);
        msgtype = dbus_message_iter_get_arg_type(&subiter);
        if (msgtype != DBUS_TYPE_ARRAY) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Argument is not Array %c! %s",
                    msgtype, name.c_str());
            DEVKITEXCEPTION("Argument is not Array");
        }

        do {
            try {
                retval.push_back(GetString(subiter));
            } catch (cDeviceKitException &e) {
                if (msg != NULL) {
                    dbus_message_unref(msg);
                }
                mLogger->logmsg(LOGLEVEL_ERROR, "%s %s",
                                e.what(), name.c_str());
                throw;
            }
        } while (dbus_message_iter_next(&subiter));
    } catch (cDeviceKitException &e) {
        if (msg != NULL) {
            dbus_message_unref(msg);
        }
        throw;
    }
    // free reply and close connection
    dbus_message_unref(msg);

    // An empty array of strings contains one null string, so fix this
    if (!retval.empty()) {
        if (retval.front().empty()) {
            retval.clear();
        }
    }
    return retval;
}

/*
 * Get Integer property
 */
dbus_int32_t cDbusDevkit::GetDbusPropertyU (const string &path,
                                               const string &name,
                                               const string &udisk_interface,
                                               int defaultval)
                                               throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    dbus_int32_t *val = NULL;
    dbus_int32_t retval = 0;
    DBusMessageIter subiter;
    DBusMessageIter iter;

    try {
        msg = CallDbusProperty(path, name, &iter, udisk_interface);
    } catch (cDeviceKitException &e) { // Ignore "No such interface"
        return defaultval;
    }
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

/*
 * Get boolean property
 */
bool cDbusDevkit::GetDbusPropertyB (const string &path,
                                       const string &name,
                                       const string &udisk_interface,
                                       bool defaultval)
                                       throw (cDeviceKitException)
{
    DBusMessage *msg = NULL;
    dbus_bool_t retval = FALSE;
    DBusMessageIter subiter;
    DBusMessageIter iter;

    try {
        msg = CallDbusProperty(path, name, &iter, udisk_interface);
    } catch (cDeviceKitException &e) { // Ignore "No such interface"
        return defaultval;
    }
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
    DBusMessage *msg = NULL;
    DBusMessage *getmsg = NULL;
    const char *fs_type = "";
    const char *opts[] = {""};
    char *val;
    int argcnt = 0;
    string retval;
    string interface;

    try {
        if (mUDisk2) {
            interface = mService + "." + "Filesystem";

            getmsg = dbus_message_new_method_call(mService.c_str(), // target for the method call
                    path.c_str(),           // object to call on
                    interface.c_str(),      // interface to call on
                    "Mount");            // method name
            if (getmsg == NULL) {
                DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
            }
            if (dbus_error_is_set(&mErr)) {
                string errmsg = "dbus_message_new_method_call failed ";
                errmsg += mErr.message;
                dbus_error_free(&mErr);
                DEVKITEXCEPTION(errmsg);
            }
            DBusMessageIter iter1, dict;

            dbus_message_iter_init_append(getmsg, &iter1 );
            dbus_message_iter_open_container( &iter1,
                    DBUS_TYPE_ARRAY,
                    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                    DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                    &dict );

            dbus_message_iter_close_container(&iter1, &dict);
            dbus_error_init(&mErr);
            msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                    -1, &mErr);
            if (dbus_error_is_set(&mErr)) {
                string errmsg = "dbus_connection_send_with_reply failed ";
                errmsg += mErr.message;
                dbus_error_free(&mErr);
                DEVKITEXCEPTION(errmsg);
            }
        }
        else {
            interface = mService + "." + UDISKS_INTERFACE;

            getmsg = dbus_message_new_method_call(mService.c_str(), // target for the method call
                    path.c_str(),           // object to call on
                    interface.c_str(),      // interface to call on
                    "FilesystemMount");            // method name
            if (getmsg == NULL) {
                DEVKITEXCEPTION("dbus_message_new_method_call Message Null");
            }
            if (dbus_error_is_set(&mErr)) {
                string errmsg = "dbus_message_new_method_call failed ";
                errmsg += mErr.message;
                dbus_error_free(&mErr);
                DEVKITEXCEPTION(errmsg);
            }

            if (! dbus_message_append_args(getmsg, DBUS_TYPE_STRING, &fs_type,
                    DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, opts, &argcnt,
                    DBUS_TYPE_INVALID)) {
                DEVKITEXCEPTION("dbus_message_append_args failed ");
            }
        }
        msg = dbus_connection_send_with_reply_and_block(mConnSystem, getmsg,
                                                        -1, &mErr);
        if (dbus_error_is_set(&mErr)) {
            string errmsg = "dbus_connection_send_with_reply failed ";
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
#ifdef DEBUG
        mLogger->logmsg(LOGLEVEL_INFO, "Mount return value: %s", val);
#endif
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
                                 const string &name,
                                 const string &interface)
    throw (cDeviceKitException)
{
    DBusMessage *msg, *getmsg;
    char *dbusarr[] = {};

    getmsg = dbus_message_new_method_call(mService.c_str(),   // target for the method call
                                       path.c_str(),          // object to call on
                                       interface.c_str(),     // interface to call on
                                       name.c_str());         // method name
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

bool cDbusDevkit::IsMediaAvailable(const string &path)
                                                    throw (cDeviceKitException) {
    if (mUDisk2) {
        string drive = GetDbusPropertyS (path, "Drive", "Block");
        return GetDbusPropertyB (drive, "MediaAvailable", "Drive");
#ifdef DEBUG
  mLogger->logmsg(LOGLEVEL_INFO, "Drive %s", drive.c_str());
#endif
        return false;
    }
    return GetDbusPropertyB (path, "device-is-media-available", UDISKS_INTERFACE);
}

bool cDbusDevkit::IsPartition(const string &path)
                                                    throw (cDeviceKitException) {
    if (mUDisk2) {
        return GetDbusPropertyB (path, "HintPartitionable", "Block");
    }
    return GetDbusPropertyB (path, "device-is-partition", UDISKS_INTERFACE);
}

string cDbusDevkit::GetNativePath (const string &path)
                                                     throw (cDeviceKitException) {
    if (mUDisk2) {
        return GetDbusPropertyS (path, "PreferredDevice", "Block");
    }
    return GetDbusPropertyS (path, "native-path", UDISKS_INTERFACE);
}

bool cDbusDevkit::IsOpticalDisk(const string &path)
                                                    throw (cDeviceKitException) {
    if (mUDisk2) {
        string drive = GetDbusPropertyS (path, "Drive", "Block");
        string media = GetDbusPropertyS (drive, "Media", "Drive");
#ifdef DEBUG
  mLogger->logmsg(LOGLEVEL_INFO, "IsOpticalDisk %s", media.c_str());
#endif
        return (media.find("optical") != string::npos);
    }
    return GetDbusPropertyB (path, "device-is-optical-disc", UDISKS_INTERFACE);
}

stringList cDbusDevkit::GetMountPaths (const string &path)
                                                    throw (cDeviceKitException) {
    if (mUDisk2) {
        stringList mountpoints = GetDbusPropertyAS (path, "MountPoints", "Filesystem");
        return (mountpoints);
    }
    return GetDbusPropertyAS (path, "DeviceMountPaths", UDISKS_INTERFACE);
}
bool cDbusDevkit::IsMounted(const string &path)
                                                    throw (cDeviceKitException){
    if (mUDisk2) {
        return !GetMountPaths(path).empty();
    }
    return GetDbusPropertyB (path, "device-is-mounted", UDISKS_INTERFACE);
}

stringList cDbusDevkit::GetDeviceFileById (const string &path)
                throw (cDeviceKitException) {
    if (mUDisk2) {
        return GetDbusPropertyAS (path, "Id", "Block");
    }
    return GetDbusPropertyAS (path, "device-file-by-id", UDISKS_INTERFACE);
}
stringList cDbusDevkit::GetDeviceFileByPath (const string &path)
                                                    throw (cDeviceKitException) {
    if (mUDisk2) {
        return GetDbusPropertyAS (path, "Device", "Block");
    }
    return GetDbusPropertyAS (path, "device-file-by-path", UDISKS_INTERFACE);
}

string cDbusDevkit::GetDeviceFile (const string &path)
                                                   throw (cDeviceKitException) {
    if (mUDisk2) {
        return GetDbusPropertyS (path, "Device", "Block");
    }
    return GetDbusPropertyS (path, "device-file", UDISKS_INTERFACE);
}

string cDbusDevkit::GetType (const string &path) throw (cDeviceKitException) {
    if (mUDisk2) {
        return GetDbusPropertyS (path, "IdType", "Block");
    }
    return GetDbusPropertyS (path, "id-type", UDISKS_INTERFACE);
}

void cDbusDevkit::UnMount (const std::string &path)
                  throw (cDeviceKitException) {

    if (mUDisk2) {
        CallInterfaceV (path, "Unmount", "Filesystem");
    }
    CallInterfaceV (path, "FilesystemUnmount", UDISKS_INTERFACE);
}
