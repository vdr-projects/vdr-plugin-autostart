
#include "cdiotester.h"
#include <cdio/cdda.h>

bool cCdioTester::isMedia (cMediaHandle d, cExtStringVector &keylist)
{
    CdIo_t *cdio;
    bool ismedia = TRUE;
    track_t tr;
    MEDIA_MASK_T m = d.GetMediaMask();

    if (!(m & MEDIA_OPTICAL)) {
        return (false);
    }
    if (!(m & MEDIA_AVAILABLE))
    {
        return (false);
    }
    cdio = cdio_open(d.GetDeviceFile().c_str(), DRIVER_DEVICE);
    if (cdio == NULL) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Can not open %s",
                                         d.GetNativePath().c_str());
        return false;
    }

    tr = cdio_get_first_track_num(cdio);
    if (tr == CDIO_INVALID_TRACK) {
        ismedia = false;
    }
    else
    {
        track_format_t track_format = cdio_get_track_format(cdio, tr);
        if (track_format != TRACK_FORMAT_AUDIO) {
           ismedia = false;
        }
    }
    cdio_destroy(cdio);
    if (ismedia) {
        keylist = mKeylist;
    }
    return (ismedia);
}
