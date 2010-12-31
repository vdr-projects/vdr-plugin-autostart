/*
 * vdrlogger.h: Logging for the media detection framework which can be used
 *              for the vdr logging mechanism.
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef VDRLOGGER_H_
#define VDRLOGGER_H_

#include "detector/logger.h"
#include <vdr/tools.h>
#include <vdr/thread.h>

// Logging Class using the VDR-Logging facility
class cVdrLogger :public cLogger {
private:
    static const int MAXSYSLOGBUF = 256;
public:
    virtual void logmsg (LOG_LEVEL severity, const char *format, ...) {
        if (SysLogLevel > severity) {
             char fmt[MAXSYSLOGBUF];
             snprintf(fmt, sizeof(fmt), "[%d] %s", cThread::ThreadId(), format);
            va_list ap;
            va_start(ap, format);
            vsyslog(LOG_ERR,format, ap);
            va_end(ap);
        }
    }
};

#endif
