#include "UtilLogAvcodec.h"
#include <sstream>
#include "UtilLog.h"

#pragma warning ( disable : 4005 ) // Redefinition of INTMAX_C/UINTMAX_C by boost and ffmpeg

extern "C" {
#include <avformat.h>
};


std::ostream& operator<< (std::ostream& os, const AVRational& obj)
{
    os << obj.num << "/" << obj.den;
    return os;
}

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj)
{
    switch (obj)
    {
    case PIX_FMT_NONE: os << "PIX_FMT_NONE"; break;
    case PIX_FMT_YUV420P: os << "PIX_FMT_YUV420P"; break;
    case PIX_FMT_YUYV422: os << "PIX_FMT_YUYV422"; break;
    case PIX_FMT_RGB24: os << "PIX_FMT_RGB24"; break;
    case PIX_FMT_BGR24: os << "PIX_FMT_BGR24"; break;
    case PIX_FMT_YUV422P: os << "PIX_FMT_YUV422P"; break;
    case PIX_FMT_YUV444P: os << "PIX_FMT_YUV444P"; break;
    case PIX_FMT_RGB32: os << "PIX_FMT_RGB32"; break;
    case PIX_FMT_YUV410P: os << "PIX_FMT_YUV410P"; break;
    case PIX_FMT_YUV411P: os << "PIX_FMT_YUV411P"; break;
    case PIX_FMT_RGB565: os << "PIX_FMT_RGB565"; break;
    case PIX_FMT_RGB555: os << "PIX_FMT_RGB555"; break;
    case PIX_FMT_GRAY8: os << "PIX_FMT_GRAY8"; break;
    case PIX_FMT_MONOWHITE: os << "PIX_FMT_MONOWHITE"; break;
    case PIX_FMT_MONOBLACK: os << "PIX_FMT_MONOBLACK"; break;
    case PIX_FMT_PAL8: os << "PIX_FMT_PAL8"; break;
    case PIX_FMT_YUVJ420P: os << "PIX_FMT_YUVJ420P"; break;
    case PIX_FMT_YUVJ422P: os << "PIX_FMT_YUVJ422P"; break;
    case PIX_FMT_YUVJ444P: os << "PIX_FMT_YUVJ444P"; break;
    case PIX_FMT_XVMC_MPEG2_MC: os << "PIX_FMT_XVMC_MPEG2_MC"; break;
    case PIX_FMT_XVMC_MPEG2_IDCT: os << "PIX_FMT_XVMC_MPEG2_IDCT"; break;
    case PIX_FMT_UYVY422: os << "PIX_FMT_UYVY422"; break;
    case PIX_FMT_UYYVYY411: os << "PIX_FMT_UYYVYY411"; break;
    case PIX_FMT_BGR32: os << "PIX_FMT_BGR32"; break;
    case PIX_FMT_BGR565: os << "PIX_FMT_BGR565"; break;
    case PIX_FMT_BGR555: os << "PIX_FMT_BGR555"; break;
    case PIX_FMT_BGR8: os << "PIX_FMT_BGR8"; break;
    case PIX_FMT_BGR4: os << "PIX_FMT_BGR4"; break;
    case PIX_FMT_BGR4_BYTE: os << "PIX_FMT_BGR4_BYTE"; break;
    case PIX_FMT_RGB8: os << "PIX_FMT_RGB8"; break;
    case PIX_FMT_RGB4: os << "PIX_FMT_RGB4"; break;
    case PIX_FMT_RGB4_BYTE: os << "PIX_FMT_RGB4_BYTE"; break;
    case PIX_FMT_NV12: os << "PIX_FMT_NV12"; break;
    case PIX_FMT_NV21: os << "PIX_FMT_NV21"; break;
    case PIX_FMT_RGB32_1: os << "PIX_FMT_RGB32_1"; break;
    case PIX_FMT_BGR32_1: os << "PIX_FMT_BGR32_1"; break;
    case PIX_FMT_GRAY16LE: os << "PIX_FMT_GRAY16LE"; break;
    case PIX_FMT_YUV440P: os << "PIX_FMT_YUV440P"; break;
    case PIX_FMT_YUVJ440P: os << "PIX_FMT_YUVJ440P"; break;
    case PIX_FMT_YUVA420P: os << "PIX_FMT_YUVA420P"; break;
    case PIX_FMT_VDPAU_H264: os << "PIX_FMT_VDPAU_H264"; break;
    case PIX_FMT_NB: os << "PIX_FMT_NB"; break;
    default: os << "Unknown PixelFormat";
    }
    return os;

};

std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj)
{
    /** /todo skip_frame in order to skip video frames? */
    os  << "["
        << "pix_fmt="                   << obj->pix_fmt                 << ","
        << "time_base="                 << obj->time_base               << ","
        << "width="                     << obj->width                   << ","
        << "height="                    << obj->height                  << ","
        << "bitrate="                   << obj->bit_rate                << ","
        << "sample_rate="               << obj->sample_rate             << ","
        << "channels="                  << obj->channels                << ","
        << "sample_fmt="                << obj->sample_fmt              << ","
        << "codec_name="                << obj->codec_name              << ","
        << "has_b_frames="              << obj->has_b_frames            << ","
        << "sample_aspect_ratio="       << obj->sample_aspect_ratio     << ","
        << "debug="                     << obj->debug                   << ","
        << "lowres="                    << obj->lowres                  << "," /** /todo use for preview rendering */
        << "coded_width="               << obj->coded_width             << "," /** /todo Set by user before init if known. Codec should override / dynamically change if needed */
        << "coded_height="              << obj->coded_height            << "," /** /todo Set by user before init if known. Codec should override / dynamically change if needed */
        << "request_channel_layout="    << obj->request_channel_layout  << "," /** /todo always stereo? */
        << "hwaccel="                   << obj->hwaccel                 << ","
        << "ticks_per_frame="           << obj->ticks_per_frame         << ","
        << "]";
    return os;
}

void Avcodec::log(void *ptr, int val, const char * msg, va_list ap)
{
    static const int nChars = 500;
    static char* fixedbuffer = new char[nChars];

    int len = vsnprintf(fixedbuffer, nChars, msg, ap);
//    int len = _vscprintf( msg, ap );
//    char* buffer = new char[len+1]; // _vscprintf doesn't count terminating '\0'
//    vsprintf(buffer,msg,ap);
    //va_end(ap); //TODO is this needed? See http://www.tin.org/bin/man.cgi?section=3&topic=vsnprintf
    if ( len > 0 && fixedbuffer[len-1] == '\n' )
    {
        // Strip new line in logged line
        fixedbuffer[len-1] = '.';
    }

    std::ostringstream o;
    if (ptr)
    {
        o   << "["
            << (*(AVClass**)ptr)->item_name(ptr)
            << ";"
            << (*(AVClass**)ptr)->class_name
            << "]";
    }
    else
    {
        o << "";
    }
    LOG_DETAIL << o.str();// << " [" << fixedbuffer << "]";
}

void Avcodec::init()
{
    av_register_all();
    url_set_interrupt_cb(0);
    av_log_set_callback( Avcodec::log );
}

void Avcodec::configureLog()
{
    if (Log::sReportingLevel == logDETAIL)
    {
        av_log_set_level(AV_LOG_VERBOSE);
    }
    else if (Log::sReportingLevel == logVIDEO)
    {
        av_log_set_level(AV_LOG_INFO);
    }
    else if (Log::sReportingLevel == logAUDIO)
    {
        av_log_set_level(AV_LOG_INFO);
    }
    else
    {
        av_log_set_level(AV_LOG_WARNING);
    }
}
