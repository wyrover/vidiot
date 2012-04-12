#include "UtilInitAvcodec.h"

#include "Config.h"
#include "UtilEnum.h"
#include "UtilLog.h"
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <map>
#include <sstream>
#include <utility>
#include <wx/intl.h>

#pragma warning ( disable : 4005 ) // Redefinition of INTMAX_C/UINTMAX_C by boost and ffmpeg
#pragma warning ( disable : 4244 ) // Conversion from int64 to int32 in method that explicitly does so.
extern "C" {
#include <avformat.h>
};

// NOTE: First value is the default
typedef std::pair<wxString, int> LevelString;
const std::list<LevelString> sLogLevels = boost::assign::list_of
    (std::make_pair(_("None")   ,AV_LOG_QUIET))
    (std::make_pair(_("Fatal")  ,AV_LOG_FATAL))
    (std::make_pair(_("Error")  ,AV_LOG_ERROR))
    (std::make_pair(_("Warning"),AV_LOG_WARNING))
    (std::make_pair(_("Info")   ,AV_LOG_INFO))
    (std::make_pair(_("Verbose"),AV_LOG_VERBOSE));

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const AVRational& obj)
{
    os << obj.num << "/" << obj.den;
    return os;
}

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj)
{
    switch (obj)
    {
    case PIX_FMT_NONE:              os << "PIX_FMT_NONE";               break;
    case PIX_FMT_YUV420P:           os << "PIX_FMT_YUV420P";            break;
    case PIX_FMT_YUYV422:           os << "PIX_FMT_YUYV422";            break;
    case PIX_FMT_RGB24:             os << "PIX_FMT_RGB24";              break;
    case PIX_FMT_BGR24:             os << "PIX_FMT_BGR24";              break;
    case PIX_FMT_YUV422P:           os << "PIX_FMT_YUV422P";            break;
    case PIX_FMT_YUV444P:           os << "PIX_FMT_YUV444P";            break;
    case PIX_FMT_RGB32:             os << "PIX_FMT_RGB32";              break;
    case PIX_FMT_YUV410P:           os << "PIX_FMT_YUV410P";            break;
    case PIX_FMT_YUV411P:           os << "PIX_FMT_YUV411P";            break;
    case PIX_FMT_RGB565:            os << "PIX_FMT_RGB565";             break;
    case PIX_FMT_RGB555:            os << "PIX_FMT_RGB555";             break;
    case PIX_FMT_GRAY8:             os << "PIX_FMT_GRAY8";              break;
    case PIX_FMT_MONOWHITE:         os << "PIX_FMT_MONOWHITE";          break;
    case PIX_FMT_MONOBLACK:         os << "PIX_FMT_MONOBLACK";          break;
    case PIX_FMT_PAL8:              os << "PIX_FMT_PAL8";               break;
    case PIX_FMT_YUVJ420P:          os << "PIX_FMT_YUVJ420P";           break;
    case PIX_FMT_YUVJ422P:          os << "PIX_FMT_YUVJ422P";           break;
    case PIX_FMT_YUVJ444P:          os << "PIX_FMT_YUVJ444P";           break;
    case PIX_FMT_XVMC_MPEG2_MC:     os << "PIX_FMT_XVMC_MPEG2_MC";      break;
    case PIX_FMT_XVMC_MPEG2_IDCT:   os << "PIX_FMT_XVMC_MPEG2_IDCT";    break;
    case PIX_FMT_UYVY422:           os << "PIX_FMT_UYVY422";            break;
    case PIX_FMT_UYYVYY411:         os << "PIX_FMT_UYYVYY411";          break;
    case PIX_FMT_BGR32:             os << "PIX_FMT_BGR32";              break;
    case PIX_FMT_BGR565:            os << "PIX_FMT_BGR565";             break;
    case PIX_FMT_BGR555:            os << "PIX_FMT_BGR555";             break;
    case PIX_FMT_BGR8:              os << "PIX_FMT_BGR8";               break;
    case PIX_FMT_BGR4:              os << "PIX_FMT_BGR4";               break;
    case PIX_FMT_BGR4_BYTE:         os << "PIX_FMT_BGR4_BYTE";          break;
    case PIX_FMT_RGB8:              os << "PIX_FMT_RGB8";               break;
    case PIX_FMT_RGB4:              os << "PIX_FMT_RGB4";               break;
    case PIX_FMT_RGB4_BYTE:         os << "PIX_FMT_RGB4_BYTE";          break;
    case PIX_FMT_NV12:              os << "PIX_FMT_NV12";               break;
    case PIX_FMT_NV21:              os << "PIX_FMT_NV21";               break;
    case PIX_FMT_RGB32_1:           os << "PIX_FMT_RGB32_1";            break;
    case PIX_FMT_BGR32_1:           os << "PIX_FMT_BGR32_1";            break;
    case PIX_FMT_GRAY16LE:          os << "PIX_FMT_GRAY16LE";           break;
    case PIX_FMT_YUV440P:           os << "PIX_FMT_YUV440P";            break;
    case PIX_FMT_YUVJ440P:          os << "PIX_FMT_YUVJ440P";           break;
    case PIX_FMT_YUVA420P:          os << "PIX_FMT_YUVA420P";           break;
    case PIX_FMT_VDPAU_H264:        os << "PIX_FMT_VDPAU_H264";         break;
    case PIX_FMT_NB:                os << "PIX_FMT_NB";                 break;
    default:                         os << "Unknown PixelFormat";
    }
    return os;
};

std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj)
{
    if (obj)
    {
        os  << '{'
            << "pix_fmt="                   << obj->pix_fmt                 << ','
            << "time_base="                 << obj->time_base               << ','
            << "width="                     << obj->width                   << ','
            << "height="                    << obj->height                  << ','
            << "bitrate="                   << obj->bit_rate                << ','
            << "sample_rate="               << obj->sample_rate             << ','
            << "channels="                  << obj->channels                << ','
            << "sample_fmt="                << obj->sample_fmt              << ','
            << "codec_name="                << obj->codec_name              << ','
            << "has_b_frames="              << obj->has_b_frames            << ','
            << "sample_aspect_ratio="       << obj->sample_aspect_ratio     << ','
            << "debug="                     << obj->debug                   << ','
            << "lowres="                    << obj->lowres                  << ','
            << "coded_width="               << obj->coded_width             << ','
            << "coded_height="              << obj->coded_height            << ','
            << "request_channel_layout="    << obj->request_channel_layout  << ','
            << "hwaccel="                   << obj->hwaccel                 << ','
            << "ticks_per_frame="           << obj->ticks_per_frame
            << '}';
    }
    else
    {
        os << '0';
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const AVFormatContext* obj)
{
    if (obj)
    {
        os  << '{';
        if (obj->av_class)
        {
            os  << "avclass="           << obj->av_class->class_name        << ',';
        }
        if (obj->iformat)
        {
            os  << "iformat.name="      << obj->iformat->name               << ','
                << "iformat.long_name=" << obj->iformat->long_name          << ',';
        }
        if (obj->oformat)
        {
            os  << "oformat.name="      << obj->oformat->name               << ','
                << "oformat.long_name=" << obj->oformat->long_name          << ',';
        }
        os  << "nb_streams="            << obj->nb_streams                  << ','
            << "filename="              << obj->filename                    << ','
            << "timestamp="             << obj->timestamp                   << ','
            << "ctx_flags="             << obj->ctx_flags                   << ','
            << "start_time="            << obj->start_time                  << ','
            << "duration="              << obj->duration                    << ','
            << "file_size="             << obj->file_size                   << ','
            << "bit_rate="              << obj->bit_rate                    << ','
            << "data_offset="           << obj->data_offset                 << ','
            << "mux_rate="              << obj->mux_rate                    << ','
            << "packet_size="           << obj->packet_size                 << ','
            << "preload="               << obj->preload                     << ','
            << "max_delay="             << obj->max_delay                   << ','
            << "loop_output="           << obj->loop_output                 << ','
            << "flags="                 << obj->flags                       << ','
            << "loop_input="            << obj->loop_input                  << ','
            << "probesize="             << obj->probesize                   << ','
            << "max_analyze_duration="  << obj->max_analyze_duration        << ','
            << "video_codec_id="        << obj->video_codec_id              << ','
            << "audio_codec_id="        << obj->audio_codec_id              << ','
            << "subtitle_codec_id="     << obj->subtitle_codec_id           << ','
            << "max_index_size="        << obj->max_index_size              << ','
            << "max_picture_buffer="    << obj->max_picture_buffer          << ','
            << "nb_chapters="           << obj->nb_chapters                 << ','
            << "debug="                 << obj->debug                       << ','
            << "start_time_realtime="   << obj->start_time_realtime
            << '}';
    }
    else
    {
        os << '0';
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const AVStream* obj)
{
    if (obj)
    {
        os  << '{'
            << "index="                         << obj->index                           << ','
            << "id="                            << obj->id                              << ','
            << "codec="                         << obj->codec                           << ','
            << "r_frame_rate="                  << obj->r_frame_rate                    << ','
            << "time_base="                     << obj->time_base                       << ','
            << "pts_wrap_bits="                 << obj->pts_wrap_bits                   << ','
            << "stream_copy="                   << obj->stream_copy                     << ','
            << "discard="                       << obj->discard                         << ','
            << "quality="                       << obj->quality                         << ','
            << "start_time="                    << obj->start_time                      << ','
            << "duration="                      << obj->duration                        << ','
            << "cur_dts="                       << obj->cur_dts                         << ','
            << "last_IP_duration="              << obj->last_IP_duration                << ','
            << "last_IP_pts="                   << obj->last_IP_pts                     << ','
            << "nb_index_entries="              << obj->nb_index_entries                << ','
            << "index_entries_allocated_size="  << obj->index_entries_allocated_size    << ','
            << "nb_frames="                     << obj->nb_frames                       << ','
            << "disposition="                   << obj->disposition                     << ','
            << "sample_aspect_ratio="           << obj->sample_aspect_ratio             << ','
            << "reference_dts="                 << obj->reference_dts                   << ','
            << "avg_frame_rate="                << obj->avg_frame_rate                  << ','
            << "codec_info_nb_frames="          << obj->codec_info_nb_frames
            << '}';
    }
    else
    {
        os <<'0';
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const AVPacket* obj)
{
    if (obj)
    {
        os  << '{'
            << "pts="                  << obj->pts             << ','
            << "dts="                  << obj->dts             << ','
            << "size="                 << obj->size            << ','
            << "stream_index="         << obj->stream_index    << ','
            << "flags="                << obj->flags           << ','
            << "side_data_elems="      << obj->side_data_elems << ','
            << "duration="             << obj->duration        << ','
            << "pos="                  << obj->pos             << ','
            << "convergence_duration=" << obj->convergence_duration
            << '}';
    }
    else
    {
        os <<'0';
    }
    return os;
}

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

const int Avcodec::sMaxLogSize = 500;
char* Avcodec::sFixedBuffer = 0;
int Avcodec::sLevel = AV_LOG_FATAL;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void Avcodec::init()
{
    sFixedBuffer = new char[sMaxLogSize];
    av_register_all();
    url_set_interrupt_cb(0);
}

void Avcodec::exit()
{
    delete[] sFixedBuffer;
    sFixedBuffer = 0;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

//static
wxString Avcodec::getDefaultLogLevel()
{
    return (*sLogLevels.begin()).first;
}

//static
std::list<wxString> Avcodec::getLogLevels()
{
    std::list<wxString> result;
    BOOST_FOREACH( auto value, sLogLevels )
    {
        result.push_back(value.first);
    }
    return result;
}

void Avcodec::configureLog()
{
    BOOST_FOREACH( auto value, sLogLevels )
    {
        if (value.first.IsSameAs(gui::Config::ReadString(gui::Config::sPathLogLevelAvcodec)))
        {
            sLevel = value.second;
            break;
        }
    }
    av_log_set_level(sLevel); // Only required for default avcodec log method
    av_log_set_callback(Avcodec::log);
}

std::string Avcodec::getErrorMessage(int errorcode)
{
    static const int errbuf_size = 256;
    char errbuf[errbuf_size];
    int errorDecodeResult = av_strerror(errorcode, errbuf, errbuf_size);
    VAR_ERROR(errorDecodeResult);
    return str( boost::format("'%1%'") % errbuf );
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Avcodec::log(void *ptr, int level, const char * msg, va_list ap)
{
    if (level > sLevel) return;
    int len = vsnprintf(sFixedBuffer, sMaxLogSize, msg, ap);
    if ( len > 0 && sFixedBuffer[len-1] == '\n' )
    {
        // Strip new line in logged line
        sFixedBuffer[len-1] = '.';
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
    Log().get("AVCODEC") << o.str() << " [" << sFixedBuffer << "]";
}