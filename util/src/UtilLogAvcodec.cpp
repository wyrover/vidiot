// Copyright 2013-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Config.h"

#define LOGENUMVALUE(value) case value: os << #value; break

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AVRational& obj)
{
    os << obj.num << "/" << obj.den;
    return os;
}

std::ostream& operator<<(std::ostream& os, const AVPixelFormat& obj)
{
    switch (obj)
    {
        LOGENUMVALUE(AV_PIX_FMT_NONE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P);
        LOGENUMVALUE(AV_PIX_FMT_YUYV422);
        LOGENUMVALUE(AV_PIX_FMT_RGB24);
        LOGENUMVALUE(AV_PIX_FMT_BGR24);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P);
        LOGENUMVALUE(AV_PIX_FMT_YUV410P);
        LOGENUMVALUE(AV_PIX_FMT_YUV411P);
        LOGENUMVALUE(AV_PIX_FMT_GRAY8);
        LOGENUMVALUE(AV_PIX_FMT_MONOWHITE);
        LOGENUMVALUE(AV_PIX_FMT_MONOBLACK);
        LOGENUMVALUE(AV_PIX_FMT_PAL8);
        LOGENUMVALUE(AV_PIX_FMT_YUVJ420P);
        LOGENUMVALUE(AV_PIX_FMT_YUVJ422P);
        LOGENUMVALUE(AV_PIX_FMT_YUVJ444P);
        LOGENUMVALUE(AV_PIX_FMT_XVMC_MPEG2_MC);
        LOGENUMVALUE(AV_PIX_FMT_XVMC_MPEG2_IDCT);
        LOGENUMVALUE(AV_PIX_FMT_UYVY422);
        LOGENUMVALUE(AV_PIX_FMT_UYYVYY411);
        LOGENUMVALUE(AV_PIX_FMT_BGR8);
        LOGENUMVALUE(AV_PIX_FMT_BGR4);
        LOGENUMVALUE(AV_PIX_FMT_BGR4_BYTE);
        LOGENUMVALUE(AV_PIX_FMT_RGB8);
        LOGENUMVALUE(AV_PIX_FMT_RGB4);
        LOGENUMVALUE(AV_PIX_FMT_RGB4_BYTE);
        LOGENUMVALUE(AV_PIX_FMT_NV12);
        LOGENUMVALUE(AV_PIX_FMT_NV21);
        LOGENUMVALUE(AV_PIX_FMT_ARGB);
        LOGENUMVALUE(AV_PIX_FMT_RGBA);
        LOGENUMVALUE(AV_PIX_FMT_ABGR);
        LOGENUMVALUE(AV_PIX_FMT_BGRA);
        LOGENUMVALUE(AV_PIX_FMT_GRAY16BE);
        LOGENUMVALUE(AV_PIX_FMT_GRAY16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV440P);
        LOGENUMVALUE(AV_PIX_FMT_YUVJ440P);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU_H264);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU_MPEG1);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU_MPEG2);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU_WMV3);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU_VC1);
        LOGENUMVALUE(AV_PIX_FMT_RGB48BE);
        LOGENUMVALUE(AV_PIX_FMT_RGB48LE);
        LOGENUMVALUE(AV_PIX_FMT_RGB565BE);
        LOGENUMVALUE(AV_PIX_FMT_RGB565LE);
        LOGENUMVALUE(AV_PIX_FMT_RGB555BE);
        LOGENUMVALUE(AV_PIX_FMT_RGB555LE);
        LOGENUMVALUE(AV_PIX_FMT_BGR565BE);
        LOGENUMVALUE(AV_PIX_FMT_BGR565LE);
        LOGENUMVALUE(AV_PIX_FMT_BGR555BE);
        LOGENUMVALUE(AV_PIX_FMT_BGR555LE);
        LOGENUMVALUE(AV_PIX_FMT_VAAPI_MOCO);
        LOGENUMVALUE(AV_PIX_FMT_VAAPI_IDCT);
        LOGENUMVALUE(AV_PIX_FMT_VAAPI_VLD);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P16BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P16BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P16BE);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU_MPEG4);
        LOGENUMVALUE(AV_PIX_FMT_DXVA2_VLD);
        LOGENUMVALUE(AV_PIX_FMT_RGB444LE);
        LOGENUMVALUE(AV_PIX_FMT_RGB444BE);
        LOGENUMVALUE(AV_PIX_FMT_BGR444LE);
        LOGENUMVALUE(AV_PIX_FMT_BGR444BE);
        LOGENUMVALUE(AV_PIX_FMT_GRAY8A);
        LOGENUMVALUE(AV_PIX_FMT_BGR48BE);
        LOGENUMVALUE(AV_PIX_FMT_BGR48LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P9BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P9LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P10BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P10LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P10BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P10LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P9BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P9LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P10BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P10LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P9BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P9LE);
        LOGENUMVALUE(AV_PIX_FMT_VDA_VLD);
        LOGENUMVALUE(AV_PIX_FMT_RGBA64BE);
        LOGENUMVALUE(AV_PIX_FMT_RGBA64LE);
        LOGENUMVALUE(AV_PIX_FMT_BGRA64BE);
        LOGENUMVALUE(AV_PIX_FMT_BGRA64LE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP);
        LOGENUMVALUE(AV_PIX_FMT_GBRP9BE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP9LE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP10BE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP10LE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP16BE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P9BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P9LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P9BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P9LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P9BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P9LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P10BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P10LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P10BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P10LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P10BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P10LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P16BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA420P16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P16BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P16BE);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P16LE);
        LOGENUMVALUE(AV_PIX_FMT_VDPAU);
        LOGENUMVALUE(AV_PIX_FMT_XYZ12LE);
        LOGENUMVALUE(AV_PIX_FMT_XYZ12BE);
        LOGENUMVALUE(AV_PIX_FMT_NV16);
        LOGENUMVALUE(AV_PIX_FMT_NV20LE);
        LOGENUMVALUE(AV_PIX_FMT_NV20BE);
        LOGENUMVALUE(AV_PIX_FMT_0RGB);
        LOGENUMVALUE(AV_PIX_FMT_RGB0);
        LOGENUMVALUE(AV_PIX_FMT_0BGR);
        LOGENUMVALUE(AV_PIX_FMT_BGR0);
        LOGENUMVALUE(AV_PIX_FMT_YUVA444P);
        LOGENUMVALUE(AV_PIX_FMT_YUVA422P);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P12BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P12LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P14BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV420P14LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P12BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P12LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P14BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV422P14LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P12BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P12LE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P14BE);
        LOGENUMVALUE(AV_PIX_FMT_YUV444P14LE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP12BE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP12LE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP14BE);
        LOGENUMVALUE(AV_PIX_FMT_GBRP14LE);
        LOGENUMVALUE(AV_PIX_FMT_GBRAP);
        LOGENUMVALUE(AV_PIX_FMT_GBRAP16BE);
        LOGENUMVALUE(AV_PIX_FMT_GBRAP16LE);
        LOGENUMVALUE(AV_PIX_FMT_YUVJ411P);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_BGGR8);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_RGGB8);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_GBRG8);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_GRBG8);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_BGGR16LE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_BGGR16BE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_RGGB16LE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_RGGB16BE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_GBRG16LE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_GBRG16BE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_GRBG16LE);
        LOGENUMVALUE(AV_PIX_FMT_BAYER_GRBG16BE);
        default: os << "Unknown AVPixelFormat (" << static_cast<long>(obj) << ")";
    }
    return os;
};

std::ostream& operator<<(std::ostream& os, const AVCodec* obj)
{
    if (obj)
    {
        os  << '{'
            << "name="           << obj->name                       << ','
            << "long_name="      << obj->long_name                  << ','
            << "type="           << static_cast<int>(obj->type)     << ','
            << "id="             << obj->id                         << ','
            << "max_lowres="     << (unsigned int)obj->max_lowres   << ','
            << "capabilities="   << obj->capabilities;
        std::vector< std::string > caps;
        if (obj->capabilities & CODEC_CAP_DELAY)               { caps.emplace_back("CODEC_CAP_DELAY");  }
        if (obj->capabilities & CODEC_CAP_SMALL_LAST_FRAME)    { caps.emplace_back("CODEC_CAP_SMALL_LAST_FRAME");  }
        if (obj->capabilities & CODEC_CAP_SUBFRAMES)           { caps.emplace_back("CODEC_CAP_SUBFRAMES");  }
        if (obj->capabilities & CODEC_CAP_CHANNEL_CONF)        { caps.emplace_back("CODEC_CAP_CHANNEL_CONF");  }
        if (obj->capabilities & CODEC_CAP_FRAME_THREADS)       { caps.emplace_back("CODEC_CAP_FRAME_THREADS");  }
        if (obj->capabilities & CODEC_CAP_SLICE_THREADS)       { caps.emplace_back("CODEC_CAP_SLICE_THREADS");  }
        if (obj->capabilities & CODEC_CAP_AUTO_THREADS)        { caps.emplace_back("CODEC_CAP_AUTO_THREADS");  }
        if (obj->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) { caps.emplace_back("CODEC_CAP_VARIABLE_FRAME_SIZE");  }
        if (obj->capabilities & CODEC_CAP_INTRA_ONLY)          { caps.emplace_back("CODEC_CAP_INTRA_ONLY");  }
        if (obj->capabilities & CODEC_CAP_LOSSLESS)            { caps.emplace_back("CODEC_CAP_LOSSLESS");  }
        if (caps.size() > 0)
        {
            os << '(';
            bool first = true;
            for ( std::string c : caps )
            {
                os << (!first ? "," : "") << c;
                first = false;
            }
            os << ')';
        }
        os << ",pix_fmts=";
        if (!obj->pix_fmts)
        {
            os << '0';
        }
        else
        {
            os << '[';
            const AVPixelFormat* f = obj->pix_fmts;
            bool first = true;
            os << (!first?",":"") << *f;
            ++f;
            os << ']';
        }
        os << '}';
    }
    else
    {
        os << '0';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const AVCodecContext* obj)
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
            << "has_b_frames="              << obj->has_b_frames            << ','
            << "sample_aspect_ratio="       << obj->sample_aspect_ratio     << ','
            << "debug="                     << obj->debug                   << ','
            << "lowres="                    << obj->lowres                  << ','
            << "coded_width="               << obj->coded_width             << ','
            << "coded_height="              << obj->coded_height            << ','
            << "request_channel_layout="    << obj->request_channel_layout  << ','
            << "hwaccel="                   << obj->hwaccel                 << ','
            << "ticks_per_frame="           << obj->ticks_per_frame         << ','
            << "codec="                     << obj->codec
            << '}';
    }
    else
    {
        os << '0';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const AVFormatContext* obj)
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
            << "ctx_flags="             << obj->ctx_flags                   << ','
            << "start_time="            << obj->start_time                  << ','
            << "duration="              << obj->duration                    << ','
            << "bit_rate="              << obj->bit_rate                    << ','
            << "packet_size="           << obj->packet_size                 << ','
            << "max_delay="             << obj->max_delay                   << ','
            << "flags="                 << obj->flags                       << ','
            << "probesize="             << obj->probesize                   << ','
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

std::ostream& operator<<(std::ostream& os, const AVStream* obj)
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
            << "discard="                       << static_cast<int>(obj->discard)       << ','
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

std::ostream& operator<<(std::ostream& os, const AVPacket* obj)
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
            << "pos="                  << obj->pos
            << '}';
    }
    else
    {
        os <<'0';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const AVOutputFormat* obj)
{
    if (obj)
    {
        os  << '{'
            << "name="           << (obj->name ? obj->name : "0")            << ','
            << "long_name="      << (obj->long_name ? obj->long_name : "0")  << ','
            << "mime_type="      << (obj->mime_type ? obj->mime_type : "0")  << ','
            << "extensions="     << (obj->extensions ? obj->extensions : "0")<< ','
            << "flags="          << obj->flags                               << ',' // AVFMT_NOFILE, AVFMT_NEEDNUMBER, AVFMT_RAWPICTURE, AVFMT_GLOBALHEADER, AVFMT_NOTIMESTAMPS, AVFMT_VARIABLE_FPS, AVFMT_NODIMENSIONS, AVFMT_NOSTREAMS
            << "priv_data_size=" << obj->priv_data_size                      << ','
            << "audio_codec="    << obj->audio_codec                         << ','
            << "video_codec="    << obj->video_codec                         << ','
            << "subtitle_codec=" << obj->subtitle_codec
            << '}';
    }
    else
    {
        os <<'0';
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const AVCodecID& obj)
{
    switch (obj)
    {
    case AV_CODEC_ID_NONE:               os << "AV_CODEC_ID_NONE";               break;
    case AV_CODEC_ID_MPEG1VIDEO:         os << "AV_CODEC_ID_MPEG1VIDEO";         break;
    case AV_CODEC_ID_MPEG2VIDEO:         os << "AV_CODEC_ID_MPEG2VIDEO";         break;
    case AV_CODEC_ID_MPEG2VIDEO_XVMC:    os << "AV_CODEC_ID_MPEG2VIDEO_XVMC";    break;
    case AV_CODEC_ID_H261:               os << "AV_CODEC_ID_H261";               break;
    case AV_CODEC_ID_H263:               os << "AV_CODEC_ID_H263";               break;
    case AV_CODEC_ID_RV10:               os << "AV_CODEC_ID_RV10";               break;
    case AV_CODEC_ID_RV20:               os << "AV_CODEC_ID_RV20";               break;
    case AV_CODEC_ID_MJPEG:              os << "AV_CODEC_ID_MJPEG";              break;
    case AV_CODEC_ID_MJPEGB:             os << "AV_CODEC_ID_MJPEGB";             break;
    case AV_CODEC_ID_LJPEG:              os << "AV_CODEC_ID_LJPEG";              break;
    case AV_CODEC_ID_SP5X:               os << "AV_CODEC_ID_SP5X";               break;
    case AV_CODEC_ID_JPEGLS:             os << "AV_CODEC_ID_JPEGLS";             break;
    case AV_CODEC_ID_MPEG4:              os << "AV_CODEC_ID_MPEG4";              break;
    case AV_CODEC_ID_RAWVIDEO:           os << "AV_CODEC_ID_RAWVIDEO";           break;
    case AV_CODEC_ID_MSMPEG4V1:          os << "AV_CODEC_ID_MSMPEG4V1";          break;
    case AV_CODEC_ID_MSMPEG4V2:          os << "AV_CODEC_ID_MSMPEG4V2";          break;
    case AV_CODEC_ID_MSMPEG4V3:          os << "AV_CODEC_ID_MSMPEG4V3";          break;
    case AV_CODEC_ID_WMV1:               os << "AV_CODEC_ID_WMV1";               break;
    case AV_CODEC_ID_WMV2:               os << "AV_CODEC_ID_WMV2";               break;
    case AV_CODEC_ID_H263P:              os << "AV_CODEC_ID_H263P";              break;
    case AV_CODEC_ID_H263I:              os << "AV_CODEC_ID_H263I";              break;
    case AV_CODEC_ID_FLV1:               os << "AV_CODEC_ID_FLV1";               break;
    case AV_CODEC_ID_SVQ1:               os << "AV_CODEC_ID_SVQ1";               break;
    case AV_CODEC_ID_SVQ3:               os << "AV_CODEC_ID_SVQ3";               break;
    case AV_CODEC_ID_DVVIDEO:            os << "AV_CODEC_ID_DVVIDEO";            break;
    case AV_CODEC_ID_HUFFYUV:            os << "AV_CODEC_ID_HUFFYUV";            break;
    case AV_CODEC_ID_CYUV:               os << "AV_CODEC_ID_CYUV";               break;
    case AV_CODEC_ID_H264:               os << "AV_CODEC_ID_H264";               break;
    case AV_CODEC_ID_H265:               os << "AV_CODEC_ID_H265";               break;
    case AV_CODEC_ID_INDEO3:             os << "AV_CODEC_ID_INDEO3";             break;
    case AV_CODEC_ID_VP3:                os << "AV_CODEC_ID_VP3";                break;
    case AV_CODEC_ID_THEORA:             os << "AV_CODEC_ID_THEORA";             break;
    case AV_CODEC_ID_ASV1:               os << "AV_CODEC_ID_ASV1";               break;
    case AV_CODEC_ID_ASV2:               os << "AV_CODEC_ID_ASV2";               break;
    case AV_CODEC_ID_FFV1:               os << "AV_CODEC_ID_FFV1";               break;
    case AV_CODEC_ID_4XM:                os << "AV_CODEC_ID_4XM";                break;
    case AV_CODEC_ID_VCR1:               os << "AV_CODEC_ID_VCR1";               break;
    case AV_CODEC_ID_CLJR:               os << "AV_CODEC_ID_CLJR";               break;
    case AV_CODEC_ID_MDEC:               os << "AV_CODEC_ID_MDEC";               break;
    case AV_CODEC_ID_ROQ:                os << "AV_CODEC_ID_ROQ";                break;
    case AV_CODEC_ID_INTERPLAY_VIDEO:    os << "AV_CODEC_ID_INTERPLAY_VIDEO";    break;
    case AV_CODEC_ID_XAN_WC3:            os << "AV_CODEC_ID_XAN_WC3";            break;
    case AV_CODEC_ID_XAN_WC4:            os << "AV_CODEC_ID_XAN_WC4";            break;
    case AV_CODEC_ID_RPZA:               os << "AV_CODEC_ID_RPZA";               break;
    case AV_CODEC_ID_CINEPAK:            os << "AV_CODEC_ID_CINEPAK";            break;
    case AV_CODEC_ID_WS_VQA:             os << "AV_CODEC_ID_WS_VQA";             break;
    case AV_CODEC_ID_MSRLE:              os << "AV_CODEC_ID_MSRLE";              break;
    case AV_CODEC_ID_MSVIDEO1:           os << "AV_CODEC_ID_MSVIDEO1";           break;
    case AV_CODEC_ID_IDCIN:              os << "AV_CODEC_ID_IDCIN";              break;
    case AV_CODEC_ID_8BPS:               os << "AV_CODEC_ID_8BPS";               break;
    case AV_CODEC_ID_SMC:                os << "AV_CODEC_ID_SMC";                break;
    case AV_CODEC_ID_FLIC:               os << "AV_CODEC_ID_FLIC";               break;
    case AV_CODEC_ID_TRUEMOTION1:        os << "AV_CODEC_ID_TRUEMOTION1";        break;
    case AV_CODEC_ID_VMDVIDEO:           os << "AV_CODEC_ID_VMDVIDEO";           break;
    case AV_CODEC_ID_MSZH:               os << "AV_CODEC_ID_MSZH";               break;
    case AV_CODEC_ID_ZLIB:               os << "AV_CODEC_ID_ZLIB";               break;
    case AV_CODEC_ID_QTRLE:              os << "AV_CODEC_ID_QTRLE";              break;
    case AV_CODEC_ID_SNOW:               os << "AV_CODEC_ID_SNOW";               break;
    case AV_CODEC_ID_TSCC:               os << "AV_CODEC_ID_TSCC";               break;
    case AV_CODEC_ID_ULTI:               os << "AV_CODEC_ID_ULTI";               break;
    case AV_CODEC_ID_QDRAW:              os << "AV_CODEC_ID_QDRAW";              break;
    case AV_CODEC_ID_VIXL:               os << "AV_CODEC_ID_VIXL";               break;
    case AV_CODEC_ID_QPEG:               os << "AV_CODEC_ID_QPEG";               break;
    case AV_CODEC_ID_PNG:                os << "AV_CODEC_ID_PNG";                break;
    case AV_CODEC_ID_PPM:                os << "AV_CODEC_ID_PPM";                break;
    case AV_CODEC_ID_PBM:                os << "AV_CODEC_ID_PBM";                break;
    case AV_CODEC_ID_PGM:                os << "AV_CODEC_ID_PGM";                break;
    case AV_CODEC_ID_PGMYUV:             os << "AV_CODEC_ID_PGMYUV";             break;
    case AV_CODEC_ID_PAM:                os << "AV_CODEC_ID_PAM";                break;
    case AV_CODEC_ID_FFVHUFF:            os << "AV_CODEC_ID_FFVHUFF";            break;
    case AV_CODEC_ID_RV30:               os << "AV_CODEC_ID_RV30";               break;
    case AV_CODEC_ID_RV40:               os << "AV_CODEC_ID_RV40";               break;
    case AV_CODEC_ID_VC1:                os << "AV_CODEC_ID_VC1";                break;
    case AV_CODEC_ID_WMV3:               os << "AV_CODEC_ID_WMV3";               break;
    case AV_CODEC_ID_LOCO:               os << "AV_CODEC_ID_LOCO";               break;
    case AV_CODEC_ID_WNV1:               os << "AV_CODEC_ID_WNV1";               break;
    case AV_CODEC_ID_AASC:               os << "AV_CODEC_ID_AASC";               break;
    case AV_CODEC_ID_INDEO2:             os << "AV_CODEC_ID_INDEO2";             break;
    case AV_CODEC_ID_FRAPS:              os << "AV_CODEC_ID_FRAPS";              break;
    case AV_CODEC_ID_TRUEMOTION2:        os << "AV_CODEC_ID_TRUEMOTION2";        break;
    case AV_CODEC_ID_BMP:                os << "AV_CODEC_ID_BMP";                break;
    case AV_CODEC_ID_CSCD:               os << "AV_CODEC_ID_CSCD";               break;
    case AV_CODEC_ID_MMVIDEO:            os << "AV_CODEC_ID_MMVIDEO";            break;
    case AV_CODEC_ID_ZMBV:               os << "AV_CODEC_ID_ZMBV";               break;
    case AV_CODEC_ID_AVS:                os << "AV_CODEC_ID_AVS";                break;
    case AV_CODEC_ID_SMACKVIDEO:         os << "AV_CODEC_ID_SMACKVIDEO";         break;
    case AV_CODEC_ID_NUV:                os << "AV_CODEC_ID_NUV";                break;
    case AV_CODEC_ID_KMVC:               os << "AV_CODEC_ID_KMVC";               break;
    case AV_CODEC_ID_FLASHSV:            os << "AV_CODEC_ID_FLASHSV";            break;
    case AV_CODEC_ID_CAVS:               os << "AV_CODEC_ID_CAVS";               break;
    case AV_CODEC_ID_JPEG2000:           os << "AV_CODEC_ID_JPEG2000";           break;
    case AV_CODEC_ID_VMNC:               os << "AV_CODEC_ID_VMNC";               break;
    case AV_CODEC_ID_VP5:                os << "AV_CODEC_ID_VP5";                break;
    case AV_CODEC_ID_VP6:                os << "AV_CODEC_ID_VP6";                break;
    case AV_CODEC_ID_VP6F:               os << "AV_CODEC_ID_VP6F";               break;
    case AV_CODEC_ID_TARGA:              os << "AV_CODEC_ID_TARGA";              break;
    case AV_CODEC_ID_DSICINVIDEO:        os << "AV_CODEC_ID_DSICINVIDEO";        break;
    case AV_CODEC_ID_TIERTEXSEQVIDEO:    os << "AV_CODEC_ID_TIERTEXSEQVIDEO";    break;
    case AV_CODEC_ID_TIFF:               os << "AV_CODEC_ID_TIFF";               break;
    case AV_CODEC_ID_GIF:                os << "AV_CODEC_ID_GIF";                break;
    case AV_CODEC_ID_DXA:                os << "AV_CODEC_ID_DXA";                break;
    case AV_CODEC_ID_DNXHD:              os << "AV_CODEC_ID_DNXHD";              break;
    case AV_CODEC_ID_THP:                os << "AV_CODEC_ID_THP";                break;
    case AV_CODEC_ID_SGI:                os << "AV_CODEC_ID_SGI";                break;
    case AV_CODEC_ID_C93:                os << "AV_CODEC_ID_C93";                break;
    case AV_CODEC_ID_BETHSOFTVID:        os << "AV_CODEC_ID_BETHSOFTVID";        break;
    case AV_CODEC_ID_PTX:                os << "AV_CODEC_ID_PTX";                break;
    case AV_CODEC_ID_TXD:                os << "AV_CODEC_ID_TXD";                break;
    case AV_CODEC_ID_VP6A:               os << "AV_CODEC_ID_VP6A";               break;
    case AV_CODEC_ID_AMV:                os << "AV_CODEC_ID_AMV";                break;
    case AV_CODEC_ID_VB:                 os << "AV_CODEC_ID_VB";                 break;
    case AV_CODEC_ID_PCX:                os << "AV_CODEC_ID_PCX";                break;
    case AV_CODEC_ID_SUNRAST:            os << "AV_CODEC_ID_SUNRAST";            break;
    case AV_CODEC_ID_INDEO4:             os << "AV_CODEC_ID_INDEO4";             break;
    case AV_CODEC_ID_INDEO5:             os << "AV_CODEC_ID_INDEO5";             break;
    case AV_CODEC_ID_MIMIC:              os << "AV_CODEC_ID_MIMIC";              break;
    case AV_CODEC_ID_RL2:                os << "AV_CODEC_ID_RL2";                break;
    case AV_CODEC_ID_ESCAPE124:          os << "AV_CODEC_ID_ESCAPE124";          break;
    case AV_CODEC_ID_DIRAC:              os << "AV_CODEC_ID_DIRAC";              break;
    case AV_CODEC_ID_BFI:                os << "AV_CODEC_ID_BFI";                break;
    case AV_CODEC_ID_CMV:                os << "AV_CODEC_ID_CMV";                break;
    case AV_CODEC_ID_MOTIONPIXELS:       os << "AV_CODEC_ID_MOTIONPIXELS";       break;
    case AV_CODEC_ID_TGV:                os << "AV_CODEC_ID_TGV";                break;
    case AV_CODEC_ID_TGQ:                os << "AV_CODEC_ID_TGQ";                break;
    case AV_CODEC_ID_TQI:                os << "AV_CODEC_ID_TQI";                break;
    case AV_CODEC_ID_AURA:               os << "AV_CODEC_ID_AURA";               break;
    case AV_CODEC_ID_AURA2:              os << "AV_CODEC_ID_AURA2";              break;
    case AV_CODEC_ID_V210X:              os << "AV_CODEC_ID_V210X";              break;
    case AV_CODEC_ID_TMV:                os << "AV_CODEC_ID_TMV";                break;
    case AV_CODEC_ID_V210:               os << "AV_CODEC_ID_V210";               break;
    case AV_CODEC_ID_DPX:                os << "AV_CODEC_ID_DPX";                break;
    case AV_CODEC_ID_MAD:                os << "AV_CODEC_ID_MAD";                break;
    case AV_CODEC_ID_FRWU:               os << "AV_CODEC_ID_FRWU";               break;
    case AV_CODEC_ID_FLASHSV2:           os << "AV_CODEC_ID_FLASHSV2";           break;
    case AV_CODEC_ID_CDGRAPHICS:         os << "AV_CODEC_ID_CDGRAPHICS";         break;
    case AV_CODEC_ID_R210:               os << "AV_CODEC_ID_R210";               break;
    case AV_CODEC_ID_ANM:                os << "AV_CODEC_ID_ANM";                break;
    case AV_CODEC_ID_BINKVIDEO:          os << "AV_CODEC_ID_BINKVIDEO";          break;
    case AV_CODEC_ID_IFF_ILBM:           os << "AV_CODEC_ID_IFF_ILBM";           break;
    case AV_CODEC_ID_KGV1:               os << "AV_CODEC_ID_KGV1";               break;
    case AV_CODEC_ID_YOP:                os << "AV_CODEC_ID_YOP";                break;
    case AV_CODEC_ID_VP8:                os << "AV_CODEC_ID_VP8";                break;
    case AV_CODEC_ID_PICTOR:             os << "AV_CODEC_ID_PICTOR";             break;
    case AV_CODEC_ID_ANSI:               os << "AV_CODEC_ID_ANSI";               break;
    case AV_CODEC_ID_A64_MULTI:          os << "AV_CODEC_ID_A64_MULTI";          break;
    case AV_CODEC_ID_A64_MULTI5:         os << "AV_CODEC_ID_A64_MULTI5";         break;
    case AV_CODEC_ID_R10K:               os << "AV_CODEC_ID_R10K";               break;
    case AV_CODEC_ID_MXPEG:              os << "AV_CODEC_ID_MXPEG";              break;
    case AV_CODEC_ID_LAGARITH:           os << "AV_CODEC_ID_LAGARITH";           break;
    case AV_CODEC_ID_PRORES:             os << "AV_CODEC_ID_PRORES";             break;
    case AV_CODEC_ID_JV:                 os << "AV_CODEC_ID_JV";                 break;
    case AV_CODEC_ID_DFA:                os << "AV_CODEC_ID_DFA";                break;
    case AV_CODEC_ID_WMV3IMAGE:          os << "AV_CODEC_ID_WMV3IMAGE";          break;
    case AV_CODEC_ID_VC1IMAGE:           os << "AV_CODEC_ID_VC1IMAGE";           break;
    case AV_CODEC_ID_BMV_VIDEO:          os << "AV_CODEC_ID_BMV_VIDEO";          break;
    case AV_CODEC_ID_VBLE:               os << "AV_CODEC_ID_VBLE";               break;
    case AV_CODEC_ID_DXTORY:             os << "AV_CODEC_ID_DXTORY";             break;
    case AV_CODEC_ID_V410:               os << "AV_CODEC_ID_V410";               break;
    case AV_CODEC_ID_Y41P:               os << "AV_CODEC_ID_Y41P";               break;
    case AV_CODEC_ID_UTVIDEO:            os << "AV_CODEC_ID_UTVIDEO";            break;
    case AV_CODEC_ID_ESCAPE130:          os << "AV_CODEC_ID_ESCAPE130";          break;
    case AV_CODEC_ID_AVRP:               os << "AV_CODEC_ID_AVRP";               break;
    case AV_CODEC_ID_G2M:                os << "AV_CODEC_ID_G2M";                break;
    case AV_CODEC_ID_V308:               os << "AV_CODEC_ID_V308";               break;
    case AV_CODEC_ID_YUV4:               os << "AV_CODEC_ID_YUV4";               break;
    case AV_CODEC_ID_PCM_S16LE:          os << "AV_CODEC_ID_PCM_S16LE";          break;
    case AV_CODEC_ID_PCM_S16BE:          os << "AV_CODEC_ID_PCM_S16BE";          break;
    case AV_CODEC_ID_PCM_U16LE:          os << "AV_CODEC_ID_PCM_U16LE";          break;
    case AV_CODEC_ID_PCM_U16BE:          os << "AV_CODEC_ID_PCM_U16BE";          break;
    case AV_CODEC_ID_PCM_S8:             os << "AV_CODEC_ID_PCM_S8";             break;
    case AV_CODEC_ID_PCM_U8:             os << "AV_CODEC_ID_PCM_U8";             break;
    case AV_CODEC_ID_PCM_MULAW:          os << "AV_CODEC_ID_PCM_MULAW";          break;
    case AV_CODEC_ID_PCM_ALAW:           os << "AV_CODEC_ID_PCM_ALAW";           break;
    case AV_CODEC_ID_PCM_S32LE:          os << "AV_CODEC_ID_PCM_S32LE";          break;
    case AV_CODEC_ID_PCM_S32BE:          os << "AV_CODEC_ID_PCM_S32BE";          break;
    case AV_CODEC_ID_PCM_U32LE:          os << "AV_CODEC_ID_PCM_U32LE";          break;
    case AV_CODEC_ID_PCM_U32BE:          os << "AV_CODEC_ID_PCM_U32BE";          break;
    case AV_CODEC_ID_PCM_S24LE:          os << "AV_CODEC_ID_PCM_S24LE";          break;
    case AV_CODEC_ID_PCM_S24BE:          os << "AV_CODEC_ID_PCM_S24BE";          break;
    case AV_CODEC_ID_PCM_U24LE:          os << "AV_CODEC_ID_PCM_U24LE";          break;
    case AV_CODEC_ID_PCM_U24BE:          os << "AV_CODEC_ID_PCM_U24BE";          break;
    case AV_CODEC_ID_PCM_S24DAUD:        os << "AV_CODEC_ID_PCM_S24DAUD";        break;
    case AV_CODEC_ID_PCM_ZORK:           os << "AV_CODEC_ID_PCM_ZORK";           break;
    case AV_CODEC_ID_PCM_S16LE_PLANAR:   os << "AV_CODEC_ID_PCM_S16LE_PLANAR";   break;
    case AV_CODEC_ID_PCM_DVD:            os << "AV_CODEC_ID_PCM_DVD";            break;
    case AV_CODEC_ID_PCM_F32BE:          os << "AV_CODEC_ID_PCM_F32BE";          break;
    case AV_CODEC_ID_PCM_F32LE:          os << "AV_CODEC_ID_PCM_F32LE";          break;
    case AV_CODEC_ID_PCM_F64BE:          os << "AV_CODEC_ID_PCM_F64BE";          break;
    case AV_CODEC_ID_PCM_F64LE:          os << "AV_CODEC_ID_PCM_F64LE";          break;
    case AV_CODEC_ID_PCM_BLURAY:         os << "AV_CODEC_ID_PCM_BLURAY";         break;
    case AV_CODEC_ID_PCM_LXF:            os << "AV_CODEC_ID_PCM_LXF";            break;
    case AV_CODEC_ID_S302M:              os << "AV_CODEC_ID_S302M";              break;
    case AV_CODEC_ID_PCM_S8_PLANAR:      os << "AV_CODEC_ID_PCM_S8_PLANAR";      break;
    case AV_CODEC_ID_ADPCM_IMA_QT:       os << "AV_CODEC_ID_ADPCM_IMA_QT";       break;
    case AV_CODEC_ID_ADPCM_IMA_WAV:      os << "AV_CODEC_ID_ADPCM_IMA_WAV";      break;
    case AV_CODEC_ID_ADPCM_IMA_DK3:      os << "AV_CODEC_ID_ADPCM_IMA_DK3";      break;
    case AV_CODEC_ID_ADPCM_IMA_DK4:      os << "AV_CODEC_ID_ADPCM_IMA_DK4";      break;
    case AV_CODEC_ID_ADPCM_IMA_WS:       os << "AV_CODEC_ID_ADPCM_IMA_WS";       break;
    case AV_CODEC_ID_ADPCM_IMA_SMJPEG:   os << "AV_CODEC_ID_ADPCM_IMA_SMJPEG";   break;
    case AV_CODEC_ID_ADPCM_MS:           os << "AV_CODEC_ID_ADPCM_MS";           break;
    case AV_CODEC_ID_ADPCM_4XM:          os << "AV_CODEC_ID_ADPCM_4XM";          break;
    case AV_CODEC_ID_ADPCM_XA:           os << "AV_CODEC_ID_ADPCM_XA";           break;
    case AV_CODEC_ID_ADPCM_ADX:          os << "AV_CODEC_ID_ADPCM_ADX";          break;
    case AV_CODEC_ID_ADPCM_EA:           os << "AV_CODEC_ID_ADPCM_EA";           break;
    case AV_CODEC_ID_ADPCM_G726:         os << "AV_CODEC_ID_ADPCM_G726";         break;
    case AV_CODEC_ID_ADPCM_CT:           os << "AV_CODEC_ID_ADPCM_CT";           break;
    case AV_CODEC_ID_ADPCM_SWF:          os << "AV_CODEC_ID_ADPCM_SWF";          break;
    case AV_CODEC_ID_ADPCM_YAMAHA:       os << "AV_CODEC_ID_ADPCM_YAMAHA";       break;
    case AV_CODEC_ID_ADPCM_SBPRO_4:      os << "AV_CODEC_ID_ADPCM_SBPRO_4";      break;
    case AV_CODEC_ID_ADPCM_SBPRO_3:      os << "AV_CODEC_ID_ADPCM_SBPRO_3";      break;
    case AV_CODEC_ID_ADPCM_SBPRO_2:      os << "AV_CODEC_ID_ADPCM_SBPRO_2";      break;
    case AV_CODEC_ID_ADPCM_THP:          os << "AV_CODEC_ID_ADPCM_THP";          break;
    case AV_CODEC_ID_ADPCM_IMA_AMV:      os << "AV_CODEC_ID_ADPCM_IMA_AMV";      break;
    case AV_CODEC_ID_ADPCM_EA_R1:        os << "AV_CODEC_ID_ADPCM_EA_R1";        break;
    case AV_CODEC_ID_ADPCM_EA_R3:        os << "AV_CODEC_ID_ADPCM_EA_R3";        break;
    case AV_CODEC_ID_ADPCM_EA_R2:        os << "AV_CODEC_ID_ADPCM_EA_R2";        break;
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:  os << "AV_CODEC_ID_ADPCM_IMA_EA_SEAD";  break;
    case AV_CODEC_ID_ADPCM_IMA_EA_EACS:  os << "AV_CODEC_ID_ADPCM_IMA_EA_EACS";  break;
    case AV_CODEC_ID_ADPCM_EA_XAS:       os << "AV_CODEC_ID_ADPCM_EA_XAS";       break;
    case AV_CODEC_ID_ADPCM_EA_MAXIS_XA:  os << "AV_CODEC_ID_ADPCM_EA_MAXIS_XA";  break;
    case AV_CODEC_ID_ADPCM_IMA_ISS:      os << "AV_CODEC_ID_ADPCM_IMA_ISS";      break;
    case AV_CODEC_ID_ADPCM_G722:         os << "AV_CODEC_ID_ADPCM_G722";         break;
    case AV_CODEC_ID_AMR_NB:             os << "AV_CODEC_ID_AMR_NB";             break;
    case AV_CODEC_ID_AMR_WB:             os << "AV_CODEC_ID_AMR_WB";             break;
    case AV_CODEC_ID_RA_144:             os << "AV_CODEC_ID_RA_144";             break;
    case AV_CODEC_ID_RA_288:             os << "AV_CODEC_ID_RA_288";             break;
    case AV_CODEC_ID_ROQ_DPCM:           os << "AV_CODEC_ID_ROQ_DPCM";           break;
    case AV_CODEC_ID_INTERPLAY_DPCM:     os << "AV_CODEC_ID_INTERPLAY_DPCM";     break;
    case AV_CODEC_ID_XAN_DPCM:           os << "AV_CODEC_ID_XAN_DPCM";           break;
    case AV_CODEC_ID_SOL_DPCM:           os << "AV_CODEC_ID_SOL_DPCM";           break;
    case AV_CODEC_ID_MP2:                os << "AV_CODEC_ID_MP2";                break;
    case AV_CODEC_ID_MP3:                os << "AV_CODEC_ID_MP3";                break;
    case AV_CODEC_ID_AAC:                os << "AV_CODEC_ID_AAC";                break;
    case AV_CODEC_ID_AC3:                os << "AV_CODEC_ID_AC3";                break;
    case AV_CODEC_ID_DTS:                os << "AV_CODEC_ID_DTS";                break;
    case AV_CODEC_ID_VORBIS:             os << "AV_CODEC_ID_VORBIS";             break;
    case AV_CODEC_ID_DVAUDIO:            os << "AV_CODEC_ID_DVAUDIO";            break;
    case AV_CODEC_ID_WMAV1:              os << "AV_CODEC_ID_WMAV1";              break;
    case AV_CODEC_ID_WMAV2:              os << "AV_CODEC_ID_WMAV2";              break;
    case AV_CODEC_ID_MACE3:              os << "AV_CODEC_ID_MACE3";              break;
    case AV_CODEC_ID_MACE6:              os << "AV_CODEC_ID_MACE6";              break;
    case AV_CODEC_ID_VMDAUDIO:           os << "AV_CODEC_ID_VMDAUDIO";           break;
    case AV_CODEC_ID_FLAC:               os << "AV_CODEC_ID_FLAC";               break;
    case AV_CODEC_ID_MP3ADU:             os << "AV_CODEC_ID_MP3ADU";             break;
    case AV_CODEC_ID_MP3ON4:             os << "AV_CODEC_ID_MP3ON4";             break;
    case AV_CODEC_ID_SHORTEN:            os << "AV_CODEC_ID_SHORTEN";            break;
    case AV_CODEC_ID_ALAC:               os << "AV_CODEC_ID_ALAC";               break;
    case AV_CODEC_ID_WESTWOOD_SND1:      os << "AV_CODEC_ID_WESTWOOD_SND1";      break;
    case AV_CODEC_ID_GSM:                os << "AV_CODEC_ID_GSM";                break;
    case AV_CODEC_ID_QDM2:               os << "AV_CODEC_ID_QDM2";               break;
    case AV_CODEC_ID_COOK:               os << "AV_CODEC_ID_COOK";               break;
    case AV_CODEC_ID_TRUESPEECH:         os << "AV_CODEC_ID_TRUESPEECH";         break;
    case AV_CODEC_ID_TTA:                os << "AV_CODEC_ID_TTA";                break;
    case AV_CODEC_ID_SMACKAUDIO:         os << "AV_CODEC_ID_SMACKAUDIO";         break;
    case AV_CODEC_ID_QCELP:              os << "AV_CODEC_ID_QCELP";              break;
    case AV_CODEC_ID_WAVPACK:            os << "AV_CODEC_ID_WAVPACK";            break;
    case AV_CODEC_ID_DSICINAUDIO:        os << "AV_CODEC_ID_DSICINAUDIO";        break;
    case AV_CODEC_ID_IMC:                os << "AV_CODEC_ID_IMC";                break;
    case AV_CODEC_ID_MUSEPACK7:          os << "AV_CODEC_ID_MUSEPACK7";          break;
    case AV_CODEC_ID_MLP:                os << "AV_CODEC_ID_MLP";                break;
    case AV_CODEC_ID_GSM_MS:             os << "AV_CODEC_ID_GSM_MS";             break;
    case AV_CODEC_ID_ATRAC3:             os << "AV_CODEC_ID_ATRAC3";             break;
    case AV_CODEC_ID_VOXWARE:            os << "AV_CODEC_ID_VOXWARE";            break;
    case AV_CODEC_ID_APE:                os << "AV_CODEC_ID_APE";                break;
    case AV_CODEC_ID_NELLYMOSER:         os << "AV_CODEC_ID_NELLYMOSER";         break;
    case AV_CODEC_ID_MUSEPACK8:          os << "AV_CODEC_ID_MUSEPACK8";          break;
    case AV_CODEC_ID_SPEEX:              os << "AV_CODEC_ID_SPEEX";              break;
    case AV_CODEC_ID_WMAVOICE:           os << "AV_CODEC_ID_WMAVOICE";           break;
    case AV_CODEC_ID_WMAPRO:             os << "AV_CODEC_ID_WMAPRO";             break;
    case AV_CODEC_ID_WMALOSSLESS:        os << "AV_CODEC_ID_WMALOSSLESS";        break;
    case AV_CODEC_ID_ATRAC3P:            os << "AV_CODEC_ID_ATRAC3P";            break;
    case AV_CODEC_ID_EAC3:               os << "AV_CODEC_ID_EAC3";               break;
    case AV_CODEC_ID_SIPR:               os << "AV_CODEC_ID_SIPR";               break;
    case AV_CODEC_ID_MP1:                os << "AV_CODEC_ID_MP1";                break;
    case AV_CODEC_ID_TWINVQ:             os << "AV_CODEC_ID_TWINVQ";             break;
    case AV_CODEC_ID_TRUEHD:             os << "AV_CODEC_ID_TRUEHD";             break;
    case AV_CODEC_ID_MP4ALS:             os << "AV_CODEC_ID_MP4ALS";             break;
    case AV_CODEC_ID_ATRAC1:             os << "AV_CODEC_ID_ATRAC1";             break;
    case AV_CODEC_ID_BINKAUDIO_RDFT:     os << "AV_CODEC_ID_BINKAUDIO_RDFT";     break;
    case AV_CODEC_ID_BINKAUDIO_DCT:      os << "AV_CODEC_ID_BINKAUDIO_DCT";      break;
    case AV_CODEC_ID_AAC_LATM:           os << "AV_CODEC_ID_AAC_LATM";           break;
    case AV_CODEC_ID_QDMC:               os << "AV_CODEC_ID_QDMC";               break;
    case AV_CODEC_ID_CELT:               os << "AV_CODEC_ID_CELT";               break;
    case AV_CODEC_ID_8SVX_EXP:           os << "AV_CODEC_ID_8SVX_EXP";           break;
    case AV_CODEC_ID_8SVX_FIB:           os << "AV_CODEC_ID_8SVX_FIB";           break;
    case AV_CODEC_ID_BMV_AUDIO:          os << "AV_CODEC_ID_BMV_AUDIO";          break;
    case AV_CODEC_ID_G729:               os << "AV_CODEC_ID_G729";               break;
    case AV_CODEC_ID_G723_1:             os << "AV_CODEC_ID_G723_1";             break;
    case AV_CODEC_ID_FFWAVESYNTH:        os << "AV_CODEC_ID_FFWAVESYNTH";        break;
    case AV_CODEC_ID_DVD_SUBTITLE:       os << "AV_CODEC_ID_DVD_SUBTITLE";       break;
    case AV_CODEC_ID_DVB_SUBTITLE:       os << "AV_CODEC_ID_DVB_SUBTITLE";       break;
    case AV_CODEC_ID_TEXT:               os << "AV_CODEC_ID_TEXT";               break;
    case AV_CODEC_ID_XSUB:               os << "AV_CODEC_ID_XSUB";               break;
    case AV_CODEC_ID_SSA:                os << "AV_CODEC_ID_SSA";                break;
    case AV_CODEC_ID_MOV_TEXT:           os << "AV_CODEC_ID_MOV_TEXT";           break;
    case AV_CODEC_ID_HDMV_PGS_SUBTITLE:  os << "AV_CODEC_ID_HDMV_PGS_SUBTITLE";  break;
    case AV_CODEC_ID_DVB_TELETEXT:       os << "AV_CODEC_ID_DVB_TELETEXT";       break;
    case AV_CODEC_ID_SRT:                os << "AV_CODEC_ID_SRT";                break;
    case AV_CODEC_ID_MICRODVD:           os << "AV_CODEC_ID_MICRODVD";           break;
    case AV_CODEC_ID_BINTEXT:            os << "AV_CODEC_ID_BINTEXT";            break;
    case AV_CODEC_ID_XBIN:               os << "AV_CODEC_ID_XBIN";               break;
    case AV_CODEC_ID_IDF:                os << "AV_CODEC_ID_IDF";                break;
    case AV_CODEC_ID_PROBE:              os << "AV_CODEC_ID_PROBE";              break;
    case AV_CODEC_ID_MPEG2TS:            os << "AV_CODEC_ID_MPEG2TS";            break;
    case AV_CODEC_ID_MPEG4SYSTEMS:       os << "AV_CODEC_ID_MPEG4SYSTEMS";       break;
    case AV_CODEC_ID_FFMETADATA:         os << "AV_CODEC_ID_FFMETADATA";         break;
    default:                          os << "Unknown AVCodecID (" << static_cast<long>(obj) << ")";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const AVSampleFormat& obj)
{
    switch (obj)
    {
    case AV_SAMPLE_FMT_NONE: os << "AV_SAMPLE_FMT_NONE"; break;
    case AV_SAMPLE_FMT_U8:   os << "AV_SAMPLE_FMT_U8";   break;
    case AV_SAMPLE_FMT_S16:  os << "AV_SAMPLE_FMT_S16";  break;
    case AV_SAMPLE_FMT_S32:  os << "AV_SAMPLE_FMT_S32";  break;
    case AV_SAMPLE_FMT_FLT:  os << "AV_SAMPLE_FMT_FLT";  break;
    case AV_SAMPLE_FMT_DBL:  os << "AV_SAMPLE_FMT_DBL";  break;
    case AV_SAMPLE_FMT_U8P:  os << "AV_SAMPLE_FMT_U8P";  break;
    case AV_SAMPLE_FMT_S16P: os << "AV_SAMPLE_FMT_S16P"; break;
    case AV_SAMPLE_FMT_S32P: os << "AV_SAMPLE_FMT_S32P"; break;
    case AV_SAMPLE_FMT_FLTP: os << "AV_SAMPLE_FMT_FLTP"; break;
    case AV_SAMPLE_FMT_DBLP: os << "AV_SAMPLE_FMT_DBLP"; break;
    default:                 os << "Unknown AVSampleFormat (" << static_cast<long>(obj) << ")";
    }
    return os;
};

wxString avcodecErrorString(int errorcode)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE];
    int errorDecodeResult = av_strerror(errorcode, buffer, AV_ERROR_MAX_STRING_SIZE);
    if (errorDecodeResult != 0)
    {
        return wxString::Format("Avcodec error code not found (%d)", errorcode);
    }
    return wxString::Format("%s (%d)", buffer, errorcode );
}
