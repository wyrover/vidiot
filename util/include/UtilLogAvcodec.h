#ifndef UTIL_LOG_AVCODEC_H
#define UTIL_LOG_AVCODEC_H

#include <libavutil/pixfmt.h>

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

typedef std::pair<wxString, int> LevelString;
extern const std::list<LevelString> sLogLevels;

#endif //UTIL_LOG_AVCODEC_H