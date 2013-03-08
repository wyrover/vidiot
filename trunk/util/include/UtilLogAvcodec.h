#ifndef UTIL_LOG_AVCODEC_H
#define UTIL_LOG_AVCODEC_H

struct AVRational;
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVOutputFormat;
enum CodecID;

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj);
std::ostream& operator<< (std::ostream& os, const AVRational& obj);
std::ostream& operator<< (std::ostream& os, const AVCodec* obj);
std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj);
std::ostream& operator<< (std::ostream& os, const AVFormatContext* obj);
std::ostream& operator<< (std::ostream& os, const AVStream* obj);
std::ostream& operator<< (std::ostream& os, const AVPacket* obj);
std::ostream& operator<< (std::ostream& os, const CodecID& obj);
std::ostream& operator<< (std::ostream& os, const AVOutputFormat* obj);
std::ostream& operator<< (std::ostream& os, const AVSampleFormat& obj);

typedef std::pair<wxString, int> LevelString;

std::string avcodecErrorString(int errorcode);

#endif //UTIL_LOG_AVCODEC_H