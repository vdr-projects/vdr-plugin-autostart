#include "logger.h"
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
