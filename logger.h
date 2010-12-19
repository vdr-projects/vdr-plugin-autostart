/*
 * logger.h: Logging for the media detection framework which can be used
 *           standalone or use the vdr logging mechanism.
 *
 * To compile the testprogramm use "make detectortest"
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <vdr/tools.h>

typedef enum {
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO
} LOG_LEVEL;

// Default Logging Class for logging to stderr/stdout
class cLogger {
public:
    virtual void logmsg (LOG_LEVEL severity, const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        if (severity > LOGLEVEL_ERROR) {
            vprintf(format, ap);
            printf("\n");
        }
        else
        {
            vfprintf(stderr, format, ap);
            fprintf(stderr,"\n");
        }
        va_end(ap);
    }
};


class cVdrLogger :public cLogger {
public:
    virtual void logmsg (LOG_LEVEL severity, const char *format, ...) {
        if (SysLogLevel > severity) {
            va_list ap;
            va_start(ap, format);
            syslog_with_tid(LOG_ERR, format, ap);
            va_end(ap);
        }
    }
};

#endif /* LOGGER_H_ */
