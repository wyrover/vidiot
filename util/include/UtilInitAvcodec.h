#ifndef UTIL_INIT_AVCODEC_H
#define UTIL_INIT_AVCODEC_H

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
    // LOCKING
    //////////////////////////////////////////////////////////////////////////

    /// This mutex is needed to ensure that several non-thread-safe avcodec
    /// methods are never executed in parallel:
    /// - av_open_input_file
    /// - av_close_input_file
    /// - av_find_stream_info
    /// - avcodec_open
    /// - avcodec_close
    static boost::mutex sMutex;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    static wxString getDefaultLogLevel();
    static std::list<wxString> getLogLevels();
    static boost::bimap<int, wxString> mapAvcodecLevels;
    static void configureLog();
    static wxString getMostRecentLogLine();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static const int sMaxLogSize;
    static char* sFixedBuffer;
    static int sLevel;
    static wxString sMostRecentLogLine;

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    static void log(void *ptr, int level, const char * msg, va_list ap);
};

#endif //UTIL_INIT_AVCODEC_H