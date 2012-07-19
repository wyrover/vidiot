#ifndef UTIL_INIT_AVCODEC_H
#define UTIL_INIT_AVCODEC_H

#include <libavutil/pixfmt.h>
#include <stdarg.h> // va_list

struct AVRational;
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVOutputFormat;

class Avcodec
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static void init();
    static void exit();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    static wxString getDefaultLogLevel();
    static std::list<wxString> getLogLevels();
    static boost::bimap<int, wxString> mapAvcodecLevels;
    static void configureLog();
    static std::string getErrorMessage(int errorcode);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static const int sMaxLogSize;
    static char* sFixedBuffer;
    static int sLevel;

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    static void log(void *ptr, int level, const char * msg, va_list ap);
};

#endif //UTIL_INIT_AVCODEC_H