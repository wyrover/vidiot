#ifndef UTIL_LOG_AVCODEC_H
#define UTIL_LOG_AVCODEC_H

#include <ostream>
#include <stdarg.h> // va_list
#include "pixfmt.h"

struct AVRational;
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj);
std::ostream& operator<< (std::ostream& os, const AVRational& obj);
std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj);
std::ostream& operator<< (std::ostream& os, const AVFormatContext* obj);
std::ostream& operator<< (std::ostream& os, const AVStream* obj);

class Avcodec
{
public:
    static void init();
    static void configureLog();
private:
    static void log(void *ptr, int val, const char * msg, va_list ap);
};

#endif //UTIL_LOG_AVCODEC_H

