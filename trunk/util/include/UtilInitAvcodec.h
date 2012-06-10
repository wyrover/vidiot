#ifndef UTIL_LOG_AVCODEC_H
#define UTIL_LOG_AVCODEC_H

#include "pixfmt.h"
#include <list>
#include <ostream>
#include <stdarg.h> // va_list
#include <wx/string.h>
#include <boost/bimap.hpp>

struct AVRational;
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVOutputFormat;

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj);
std::ostream& operator<< (std::ostream& os, const AVRational& obj);
std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj);
std::ostream& operator<< (std::ostream& os, const AVFormatContext* obj);
std::ostream& operator<< (std::ostream& os, const AVStream* obj);
std::ostream& operator<< (std::ostream& os, const AVPacket* obj);
std::ostream& operator<< (std::ostream& os, const AVOutputFormat* obj);

class Avcodec
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static void init();
    static void registerOutputCodecs(); ///< Not in init, since this method also logs. And init() is called before logging is initialized properly.
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

#endif //UTIL_LOG_AVCODEC_H