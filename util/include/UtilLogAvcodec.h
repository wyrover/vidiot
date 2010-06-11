#ifndef UTIL_LOG_AVCODEC_H
#define UTIL_LOG_AVCODEC_H

#include <ostream>

enum PixelFormat;
struct AVRational;
struct AVCodecContext;

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj);
std::ostream& operator<< (std::ostream& os, const AVRational& obj);
std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj);

class Avcodec
{
public:
    static void init();
    static void configureLog();
private:
    static void log(void *ptr, int val, const char * msg, va_list ap);
};

#endif //UTIL_LOG_AVCODEC_H

