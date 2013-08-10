// Copyright 2013 Eric Raijmakers.
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

#include "UtilLogAvcodec.h"

#include "Config.h"
#include "UtilEnum.h"
#include "UtilLog.h"

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

std::ostream& operator<< (std::ostream& os, const AVCodec* obj)
{
    if (obj)
    {
        os  << '{'
            << "name="           << obj->name         << ','
            << "long_name="      << obj->long_name    << ','
            << "type="           << obj->type         << ','
            << "id="             << obj->id           << ','
            << "max_lowres="     << (unsigned int)obj->max_lowres   << ','
            << "capabilities="   << obj->capabilities;
        std::list< std::string > caps;
        if (obj->capabilities & CODEC_CAP_DELAY)               { caps.push_back("CODEC_CAP_DELAY");  }
        if (obj->capabilities & CODEC_CAP_SMALL_LAST_FRAME)    { caps.push_back("CODEC_CAP_SMALL_LAST_FRAME");  }
        if (obj->capabilities & CODEC_CAP_SUBFRAMES)           { caps.push_back("CODEC_CAP_SUBFRAMES");  }
        if (obj->capabilities & CODEC_CAP_CHANNEL_CONF)        { caps.push_back("CODEC_CAP_CHANNEL_CONF");  }
        if (obj->capabilities & CODEC_CAP_FRAME_THREADS)       { caps.push_back("CODEC_CAP_FRAME_THREADS");  }
        if (obj->capabilities & CODEC_CAP_SLICE_THREADS)       { caps.push_back("CODEC_CAP_SLICE_THREADS");  }
        if (obj->capabilities & CODEC_CAP_AUTO_THREADS)        { caps.push_back("CODEC_CAP_AUTO_THREADS");  }
        if (obj->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) { caps.push_back("CODEC_CAP_VARIABLE_FRAME_SIZE");  }
        if (obj->capabilities & CODEC_CAP_INTRA_ONLY)          { caps.push_back("CODEC_CAP_INTRA_ONLY");  }
        if (obj->capabilities & CODEC_CAP_LOSSLESS)            { caps.push_back("CODEC_CAP_LOSSLESS");  }
        if (caps.size() > 0)
        {
            os << '(';
            bool first = true;
            BOOST_FOREACH( std::string c, caps )
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
            const PixelFormat* f = obj->pix_fmts;
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
            << "ctx_flags="             << obj->ctx_flags                   << ','
            << "start_time="            << obj->start_time                  << ','
            << "duration="              << obj->duration                    << ','
            << "bit_rate="              << obj->bit_rate                    << ','
            << "data_offset="           << obj->data_offset                 << ','
            << "packet_size="           << obj->packet_size                 << ','
            << "max_delay="             << obj->max_delay                   << ','
            << "flags="                 << obj->flags                       << ','
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
            << "discard="                       << obj->discard                         << ','
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

std::ostream& operator<< (std::ostream& os, const AVOutputFormat* obj)
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

std::ostream& operator<< (std::ostream& os, const CodecID& obj)
{
    switch (obj)
    {
    case CODEC_ID_NONE:               os << "CODEC_ID_NONE";               break;
    case CODEC_ID_MPEG1VIDEO:         os << "CODEC_ID_MPEG1VIDEO";         break;
    case CODEC_ID_MPEG2VIDEO:         os << "CODEC_ID_MPEG2VIDEO";         break;
    case CODEC_ID_MPEG2VIDEO_XVMC:    os << "CODEC_ID_MPEG2VIDEO_XVMC";    break;
    case CODEC_ID_H261:               os << "CODEC_ID_H261";               break;
    case CODEC_ID_H263:               os << "CODEC_ID_H263";               break;
    case CODEC_ID_RV10:               os << "CODEC_ID_RV10";               break;
    case CODEC_ID_RV20:               os << "CODEC_ID_RV20";               break;
    case CODEC_ID_MJPEG:              os << "CODEC_ID_MJPEG";              break;
    case CODEC_ID_MJPEGB:             os << "CODEC_ID_MJPEGB";             break;
    case CODEC_ID_LJPEG:              os << "CODEC_ID_LJPEG";              break;
    case CODEC_ID_SP5X:               os << "CODEC_ID_SP5X";               break;
    case CODEC_ID_JPEGLS:             os << "CODEC_ID_JPEGLS";             break;
    case CODEC_ID_MPEG4:              os << "CODEC_ID_MPEG4";              break;
    case CODEC_ID_RAWVIDEO:           os << "CODEC_ID_RAWVIDEO";           break;
    case CODEC_ID_MSMPEG4V1:          os << "CODEC_ID_MSMPEG4V1";          break;
    case CODEC_ID_MSMPEG4V2:          os << "CODEC_ID_MSMPEG4V2";          break;
    case CODEC_ID_MSMPEG4V3:          os << "CODEC_ID_MSMPEG4V3";          break;
    case CODEC_ID_WMV1:               os << "CODEC_ID_WMV1";               break;
    case CODEC_ID_WMV2:               os << "CODEC_ID_WMV2";               break;
    case CODEC_ID_H263P:              os << "CODEC_ID_H263P";              break;
    case CODEC_ID_H263I:              os << "CODEC_ID_H263I";              break;
    case CODEC_ID_FLV1:               os << "CODEC_ID_FLV1";               break;
    case CODEC_ID_SVQ1:               os << "CODEC_ID_SVQ1";               break;
    case CODEC_ID_SVQ3:               os << "CODEC_ID_SVQ3";               break;
    case CODEC_ID_DVVIDEO:            os << "CODEC_ID_DVVIDEO";            break;
    case CODEC_ID_HUFFYUV:            os << "CODEC_ID_HUFFYUV";            break;
    case CODEC_ID_CYUV:               os << "CODEC_ID_CYUV";               break;
    case CODEC_ID_H264:               os << "CODEC_ID_H264";               break;
    case CODEC_ID_INDEO3:             os << "CODEC_ID_INDEO3";             break;
    case CODEC_ID_VP3:                os << "CODEC_ID_VP3";                break;
    case CODEC_ID_THEORA:             os << "CODEC_ID_THEORA";             break;
    case CODEC_ID_ASV1:               os << "CODEC_ID_ASV1";               break;
    case CODEC_ID_ASV2:               os << "CODEC_ID_ASV2";               break;
    case CODEC_ID_FFV1:               os << "CODEC_ID_FFV1";               break;
    case CODEC_ID_4XM:                os << "CODEC_ID_4XM";                break;
    case CODEC_ID_VCR1:               os << "CODEC_ID_VCR1";               break;
    case CODEC_ID_CLJR:               os << "CODEC_ID_CLJR";               break;
    case CODEC_ID_MDEC:               os << "CODEC_ID_MDEC";               break;
    case CODEC_ID_ROQ:                os << "CODEC_ID_ROQ";                break;
    case CODEC_ID_INTERPLAY_VIDEO:    os << "CODEC_ID_INTERPLAY_VIDEO";    break;
    case CODEC_ID_XAN_WC3:            os << "CODEC_ID_XAN_WC3";            break;
    case CODEC_ID_XAN_WC4:            os << "CODEC_ID_XAN_WC4";            break;
    case CODEC_ID_RPZA:               os << "CODEC_ID_RPZA";               break;
    case CODEC_ID_CINEPAK:            os << "CODEC_ID_CINEPAK";            break;
    case CODEC_ID_WS_VQA:             os << "CODEC_ID_WS_VQA";             break;
    case CODEC_ID_MSRLE:              os << "CODEC_ID_MSRLE";              break;
    case CODEC_ID_MSVIDEO1:           os << "CODEC_ID_MSVIDEO1";           break;
    case CODEC_ID_IDCIN:              os << "CODEC_ID_IDCIN";              break;
    case CODEC_ID_8BPS:               os << "CODEC_ID_8BPS";               break;
    case CODEC_ID_SMC:                os << "CODEC_ID_SMC";                break;
    case CODEC_ID_FLIC:               os << "CODEC_ID_FLIC";               break;
    case CODEC_ID_TRUEMOTION1:        os << "CODEC_ID_TRUEMOTION1";        break;
    case CODEC_ID_VMDVIDEO:           os << "CODEC_ID_VMDVIDEO";           break;
    case CODEC_ID_MSZH:               os << "CODEC_ID_MSZH";               break;
    case CODEC_ID_ZLIB:               os << "CODEC_ID_ZLIB";               break;
    case CODEC_ID_QTRLE:              os << "CODEC_ID_QTRLE";              break;
    case CODEC_ID_SNOW:               os << "CODEC_ID_SNOW";               break;
    case CODEC_ID_TSCC:               os << "CODEC_ID_TSCC";               break;
    case CODEC_ID_ULTI:               os << "CODEC_ID_ULTI";               break;
    case CODEC_ID_QDRAW:              os << "CODEC_ID_QDRAW";              break;
    case CODEC_ID_VIXL:               os << "CODEC_ID_VIXL";               break;
    case CODEC_ID_QPEG:               os << "CODEC_ID_QPEG";               break;
    case CODEC_ID_PNG:                os << "CODEC_ID_PNG";                break;
    case CODEC_ID_PPM:                os << "CODEC_ID_PPM";                break;
    case CODEC_ID_PBM:                os << "CODEC_ID_PBM";                break;
    case CODEC_ID_PGM:                os << "CODEC_ID_PGM";                break;
    case CODEC_ID_PGMYUV:             os << "CODEC_ID_PGMYUV";             break;
    case CODEC_ID_PAM:                os << "CODEC_ID_PAM";                break;
    case CODEC_ID_FFVHUFF:            os << "CODEC_ID_FFVHUFF";            break;
    case CODEC_ID_RV30:               os << "CODEC_ID_RV30";               break;
    case CODEC_ID_RV40:               os << "CODEC_ID_RV40";               break;
    case CODEC_ID_VC1:                os << "CODEC_ID_VC1";                break;
    case CODEC_ID_WMV3:               os << "CODEC_ID_WMV3";               break;
    case CODEC_ID_LOCO:               os << "CODEC_ID_LOCO";               break;
    case CODEC_ID_WNV1:               os << "CODEC_ID_WNV1";               break;
    case CODEC_ID_AASC:               os << "CODEC_ID_AASC";               break;
    case CODEC_ID_INDEO2:             os << "CODEC_ID_INDEO2";             break;
    case CODEC_ID_FRAPS:              os << "CODEC_ID_FRAPS";              break;
    case CODEC_ID_TRUEMOTION2:        os << "CODEC_ID_TRUEMOTION2";        break;
    case CODEC_ID_BMP:                os << "CODEC_ID_BMP";                break;
    case CODEC_ID_CSCD:               os << "CODEC_ID_CSCD";               break;
    case CODEC_ID_MMVIDEO:            os << "CODEC_ID_MMVIDEO";            break;
    case CODEC_ID_ZMBV:               os << "CODEC_ID_ZMBV";               break;
    case CODEC_ID_AVS:                os << "CODEC_ID_AVS";                break;
    case CODEC_ID_SMACKVIDEO:         os << "CODEC_ID_SMACKVIDEO";         break;
    case CODEC_ID_NUV:                os << "CODEC_ID_NUV";                break;
    case CODEC_ID_KMVC:               os << "CODEC_ID_KMVC";               break;
    case CODEC_ID_FLASHSV:            os << "CODEC_ID_FLASHSV";            break;
    case CODEC_ID_CAVS:               os << "CODEC_ID_CAVS";               break;
    case CODEC_ID_JPEG2000:           os << "CODEC_ID_JPEG2000";           break;
    case CODEC_ID_VMNC:               os << "CODEC_ID_VMNC";               break;
    case CODEC_ID_VP5:                os << "CODEC_ID_VP5";                break;
    case CODEC_ID_VP6:                os << "CODEC_ID_VP6";                break;
    case CODEC_ID_VP6F:               os << "CODEC_ID_VP6F";               break;
    case CODEC_ID_TARGA:              os << "CODEC_ID_TARGA";              break;
    case CODEC_ID_DSICINVIDEO:        os << "CODEC_ID_DSICINVIDEO";        break;
    case CODEC_ID_TIERTEXSEQVIDEO:    os << "CODEC_ID_TIERTEXSEQVIDEO";    break;
    case CODEC_ID_TIFF:               os << "CODEC_ID_TIFF";               break;
    case CODEC_ID_GIF:                os << "CODEC_ID_GIF";                break;
    case CODEC_ID_DXA:                os << "CODEC_ID_DXA";                break;
    case CODEC_ID_DNXHD:              os << "CODEC_ID_DNXHD";              break;
    case CODEC_ID_THP:                os << "CODEC_ID_THP";                break;
    case CODEC_ID_SGI:                os << "CODEC_ID_SGI";                break;
    case CODEC_ID_C93:                os << "CODEC_ID_C93";                break;
    case CODEC_ID_BETHSOFTVID:        os << "CODEC_ID_BETHSOFTVID";        break;
    case CODEC_ID_PTX:                os << "CODEC_ID_PTX";                break;
    case CODEC_ID_TXD:                os << "CODEC_ID_TXD";                break;
    case CODEC_ID_VP6A:               os << "CODEC_ID_VP6A";               break;
    case CODEC_ID_AMV:                os << "CODEC_ID_AMV";                break;
    case CODEC_ID_VB:                 os << "CODEC_ID_VB";                 break;
    case CODEC_ID_PCX:                os << "CODEC_ID_PCX";                break;
    case CODEC_ID_SUNRAST:            os << "CODEC_ID_SUNRAST";            break;
    case CODEC_ID_INDEO4:             os << "CODEC_ID_INDEO4";             break;
    case CODEC_ID_INDEO5:             os << "CODEC_ID_INDEO5";             break;
    case CODEC_ID_MIMIC:              os << "CODEC_ID_MIMIC";              break;
    case CODEC_ID_RL2:                os << "CODEC_ID_RL2";                break;
    case CODEC_ID_ESCAPE124:          os << "CODEC_ID_ESCAPE124";          break;
    case CODEC_ID_DIRAC:              os << "CODEC_ID_DIRAC";              break;
    case CODEC_ID_BFI:                os << "CODEC_ID_BFI";                break;
    case CODEC_ID_CMV:                os << "CODEC_ID_CMV";                break;
    case CODEC_ID_MOTIONPIXELS:       os << "CODEC_ID_MOTIONPIXELS";       break;
    case CODEC_ID_TGV:                os << "CODEC_ID_TGV";                break;
    case CODEC_ID_TGQ:                os << "CODEC_ID_TGQ";                break;
    case CODEC_ID_TQI:                os << "CODEC_ID_TQI";                break;
    case CODEC_ID_AURA:               os << "CODEC_ID_AURA";               break;
    case CODEC_ID_AURA2:              os << "CODEC_ID_AURA2";              break;
    case CODEC_ID_V210X:              os << "CODEC_ID_V210X";              break;
    case CODEC_ID_TMV:                os << "CODEC_ID_TMV";                break;
    case CODEC_ID_V210:               os << "CODEC_ID_V210";               break;
    case CODEC_ID_DPX:                os << "CODEC_ID_DPX";                break;
    case CODEC_ID_MAD:                os << "CODEC_ID_MAD";                break;
    case CODEC_ID_FRWU:               os << "CODEC_ID_FRWU";               break;
    case CODEC_ID_FLASHSV2:           os << "CODEC_ID_FLASHSV2";           break;
    case CODEC_ID_CDGRAPHICS:         os << "CODEC_ID_CDGRAPHICS";         break;
    case CODEC_ID_R210:               os << "CODEC_ID_R210";               break;
    case CODEC_ID_ANM:                os << "CODEC_ID_ANM";                break;
    case CODEC_ID_BINKVIDEO:          os << "CODEC_ID_BINKVIDEO";          break;
    case CODEC_ID_IFF_ILBM:           os << "CODEC_ID_IFF_ILBM";           break;
    case CODEC_ID_IFF_BYTERUN1:       os << "CODEC_ID_IFF_BYTERUN1";       break;
    case CODEC_ID_KGV1:               os << "CODEC_ID_KGV1";               break;
    case CODEC_ID_YOP:                os << "CODEC_ID_YOP";                break;
    case CODEC_ID_VP8:                os << "CODEC_ID_VP8";                break;
    case CODEC_ID_PICTOR:             os << "CODEC_ID_PICTOR";             break;
    case CODEC_ID_ANSI:               os << "CODEC_ID_ANSI";               break;
    case CODEC_ID_A64_MULTI:          os << "CODEC_ID_A64_MULTI";          break;
    case CODEC_ID_A64_MULTI5:         os << "CODEC_ID_A64_MULTI5";         break;
    case CODEC_ID_R10K:               os << "CODEC_ID_R10K";               break;
    case CODEC_ID_MXPEG:              os << "CODEC_ID_MXPEG";              break;
    case CODEC_ID_LAGARITH:           os << "CODEC_ID_LAGARITH";           break;
    case CODEC_ID_PRORES:             os << "CODEC_ID_PRORES";             break;
    case CODEC_ID_JV:                 os << "CODEC_ID_JV";                 break;
    case CODEC_ID_DFA:                os << "CODEC_ID_DFA";                break;
    case CODEC_ID_WMV3IMAGE:          os << "CODEC_ID_WMV3IMAGE";          break;
    case CODEC_ID_VC1IMAGE:           os << "CODEC_ID_VC1IMAGE";           break;
    case CODEC_ID_BMV_VIDEO:          os << "CODEC_ID_BMV_VIDEO";          break;
    case CODEC_ID_VBLE:               os << "CODEC_ID_VBLE";               break;
    case CODEC_ID_DXTORY:             os << "CODEC_ID_DXTORY";             break;
    case CODEC_ID_V410:               os << "CODEC_ID_V410";               break;
    case CODEC_ID_Y41P:               os << "CODEC_ID_Y41P";               break;
    case CODEC_ID_UTVIDEO:            os << "CODEC_ID_UTVIDEO";            break;
    case CODEC_ID_ESCAPE130:          os << "CODEC_ID_ESCAPE130";          break;
    case CODEC_ID_AVRP:               os << "CODEC_ID_AVRP";               break;
    case CODEC_ID_G2M:                os << "CODEC_ID_G2M";                break;
    case CODEC_ID_V308:               os << "CODEC_ID_V308";               break;
    case CODEC_ID_YUV4:               os << "CODEC_ID_YUV4";               break;
    case CODEC_ID_PCM_S16LE:          os << "CODEC_ID_PCM_S16LE";          break;
    case CODEC_ID_PCM_S16BE:          os << "CODEC_ID_PCM_S16BE";          break;
    case CODEC_ID_PCM_U16LE:          os << "CODEC_ID_PCM_U16LE";          break;
    case CODEC_ID_PCM_U16BE:          os << "CODEC_ID_PCM_U16BE";          break;
    case CODEC_ID_PCM_S8:             os << "CODEC_ID_PCM_S8";             break;
    case CODEC_ID_PCM_U8:             os << "CODEC_ID_PCM_U8";             break;
    case CODEC_ID_PCM_MULAW:          os << "CODEC_ID_PCM_MULAW";          break;
    case CODEC_ID_PCM_ALAW:           os << "CODEC_ID_PCM_ALAW";           break;
    case CODEC_ID_PCM_S32LE:          os << "CODEC_ID_PCM_S32LE";          break;
    case CODEC_ID_PCM_S32BE:          os << "CODEC_ID_PCM_S32BE";          break;
    case CODEC_ID_PCM_U32LE:          os << "CODEC_ID_PCM_U32LE";          break;
    case CODEC_ID_PCM_U32BE:          os << "CODEC_ID_PCM_U32BE";          break;
    case CODEC_ID_PCM_S24LE:          os << "CODEC_ID_PCM_S24LE";          break;
    case CODEC_ID_PCM_S24BE:          os << "CODEC_ID_PCM_S24BE";          break;
    case CODEC_ID_PCM_U24LE:          os << "CODEC_ID_PCM_U24LE";          break;
    case CODEC_ID_PCM_U24BE:          os << "CODEC_ID_PCM_U24BE";          break;
    case CODEC_ID_PCM_S24DAUD:        os << "CODEC_ID_PCM_S24DAUD";        break;
    case CODEC_ID_PCM_ZORK:           os << "CODEC_ID_PCM_ZORK";           break;
    case CODEC_ID_PCM_S16LE_PLANAR:   os << "CODEC_ID_PCM_S16LE_PLANAR";   break;
    case CODEC_ID_PCM_DVD:            os << "CODEC_ID_PCM_DVD";            break;
    case CODEC_ID_PCM_F32BE:          os << "CODEC_ID_PCM_F32BE";          break;
    case CODEC_ID_PCM_F32LE:          os << "CODEC_ID_PCM_F32LE";          break;
    case CODEC_ID_PCM_F64BE:          os << "CODEC_ID_PCM_F64BE";          break;
    case CODEC_ID_PCM_F64LE:          os << "CODEC_ID_PCM_F64LE";          break;
    case CODEC_ID_PCM_BLURAY:         os << "CODEC_ID_PCM_BLURAY";         break;
    case CODEC_ID_PCM_LXF:            os << "CODEC_ID_PCM_LXF";            break;
    case CODEC_ID_S302M:              os << "CODEC_ID_S302M";              break;
    case CODEC_ID_PCM_S8_PLANAR:      os << "CODEC_ID_PCM_S8_PLANAR";      break;
    case CODEC_ID_ADPCM_IMA_QT:       os << "CODEC_ID_ADPCM_IMA_QT";       break;
    case CODEC_ID_ADPCM_IMA_WAV:      os << "CODEC_ID_ADPCM_IMA_WAV";      break;
    case CODEC_ID_ADPCM_IMA_DK3:      os << "CODEC_ID_ADPCM_IMA_DK3";      break;
    case CODEC_ID_ADPCM_IMA_DK4:      os << "CODEC_ID_ADPCM_IMA_DK4";      break;
    case CODEC_ID_ADPCM_IMA_WS:       os << "CODEC_ID_ADPCM_IMA_WS";       break;
    case CODEC_ID_ADPCM_IMA_SMJPEG:   os << "CODEC_ID_ADPCM_IMA_SMJPEG";   break;
    case CODEC_ID_ADPCM_MS:           os << "CODEC_ID_ADPCM_MS";           break;
    case CODEC_ID_ADPCM_4XM:          os << "CODEC_ID_ADPCM_4XM";          break;
    case CODEC_ID_ADPCM_XA:           os << "CODEC_ID_ADPCM_XA";           break;
    case CODEC_ID_ADPCM_ADX:          os << "CODEC_ID_ADPCM_ADX";          break;
    case CODEC_ID_ADPCM_EA:           os << "CODEC_ID_ADPCM_EA";           break;
    case CODEC_ID_ADPCM_G726:         os << "CODEC_ID_ADPCM_G726";         break;
    case CODEC_ID_ADPCM_CT:           os << "CODEC_ID_ADPCM_CT";           break;
    case CODEC_ID_ADPCM_SWF:          os << "CODEC_ID_ADPCM_SWF";          break;
    case CODEC_ID_ADPCM_YAMAHA:       os << "CODEC_ID_ADPCM_YAMAHA";       break;
    case CODEC_ID_ADPCM_SBPRO_4:      os << "CODEC_ID_ADPCM_SBPRO_4";      break;
    case CODEC_ID_ADPCM_SBPRO_3:      os << "CODEC_ID_ADPCM_SBPRO_3";      break;
    case CODEC_ID_ADPCM_SBPRO_2:      os << "CODEC_ID_ADPCM_SBPRO_2";      break;
    case CODEC_ID_ADPCM_THP:          os << "CODEC_ID_ADPCM_THP";          break;
    case CODEC_ID_ADPCM_IMA_AMV:      os << "CODEC_ID_ADPCM_IMA_AMV";      break;
    case CODEC_ID_ADPCM_EA_R1:        os << "CODEC_ID_ADPCM_EA_R1";        break;
    case CODEC_ID_ADPCM_EA_R3:        os << "CODEC_ID_ADPCM_EA_R3";        break;
    case CODEC_ID_ADPCM_EA_R2:        os << "CODEC_ID_ADPCM_EA_R2";        break;
    case CODEC_ID_ADPCM_IMA_EA_SEAD:  os << "CODEC_ID_ADPCM_IMA_EA_SEAD";  break;
    case CODEC_ID_ADPCM_IMA_EA_EACS:  os << "CODEC_ID_ADPCM_IMA_EA_EACS";  break;
    case CODEC_ID_ADPCM_EA_XAS:       os << "CODEC_ID_ADPCM_EA_XAS";       break;
    case CODEC_ID_ADPCM_EA_MAXIS_XA:  os << "CODEC_ID_ADPCM_EA_MAXIS_XA";  break;
    case CODEC_ID_ADPCM_IMA_ISS:      os << "CODEC_ID_ADPCM_IMA_ISS";      break;
    case CODEC_ID_ADPCM_G722:         os << "CODEC_ID_ADPCM_G722";         break;
    case CODEC_ID_AMR_NB:             os << "CODEC_ID_AMR_NB";             break;
    case CODEC_ID_AMR_WB:             os << "CODEC_ID_AMR_WB";             break;
    case CODEC_ID_RA_144:             os << "CODEC_ID_RA_144";             break;
    case CODEC_ID_RA_288:             os << "CODEC_ID_RA_288";             break;
    case CODEC_ID_ROQ_DPCM:           os << "CODEC_ID_ROQ_DPCM";           break;
    case CODEC_ID_INTERPLAY_DPCM:     os << "CODEC_ID_INTERPLAY_DPCM";     break;
    case CODEC_ID_XAN_DPCM:           os << "CODEC_ID_XAN_DPCM";           break;
    case CODEC_ID_SOL_DPCM:           os << "CODEC_ID_SOL_DPCM";           break;
    case CODEC_ID_MP2:                os << "CODEC_ID_MP2";                break;
    case CODEC_ID_MP3:                os << "CODEC_ID_MP3";                break;
    case CODEC_ID_AAC:                os << "CODEC_ID_AAC";                break;
    case CODEC_ID_AC3:                os << "CODEC_ID_AC3";                break;
    case CODEC_ID_DTS:                os << "CODEC_ID_DTS";                break;
    case CODEC_ID_VORBIS:             os << "CODEC_ID_VORBIS";             break;
    case CODEC_ID_DVAUDIO:            os << "CODEC_ID_DVAUDIO";            break;
    case CODEC_ID_WMAV1:              os << "CODEC_ID_WMAV1";              break;
    case CODEC_ID_WMAV2:              os << "CODEC_ID_WMAV2";              break;
    case CODEC_ID_MACE3:              os << "CODEC_ID_MACE3";              break;
    case CODEC_ID_MACE6:              os << "CODEC_ID_MACE6";              break;
    case CODEC_ID_VMDAUDIO:           os << "CODEC_ID_VMDAUDIO";           break;
    case CODEC_ID_FLAC:               os << "CODEC_ID_FLAC";               break;
    case CODEC_ID_MP3ADU:             os << "CODEC_ID_MP3ADU";             break;
    case CODEC_ID_MP3ON4:             os << "CODEC_ID_MP3ON4";             break;
    case CODEC_ID_SHORTEN:            os << "CODEC_ID_SHORTEN";            break;
    case CODEC_ID_ALAC:               os << "CODEC_ID_ALAC";               break;
    case CODEC_ID_WESTWOOD_SND1:      os << "CODEC_ID_WESTWOOD_SND1";      break;
    case CODEC_ID_GSM:                os << "CODEC_ID_GSM";                break;
    case CODEC_ID_QDM2:               os << "CODEC_ID_QDM2";               break;
    case CODEC_ID_COOK:               os << "CODEC_ID_COOK";               break;
    case CODEC_ID_TRUESPEECH:         os << "CODEC_ID_TRUESPEECH";         break;
    case CODEC_ID_TTA:                os << "CODEC_ID_TTA";                break;
    case CODEC_ID_SMACKAUDIO:         os << "CODEC_ID_SMACKAUDIO";         break;
    case CODEC_ID_QCELP:              os << "CODEC_ID_QCELP";              break;
    case CODEC_ID_WAVPACK:            os << "CODEC_ID_WAVPACK";            break;
    case CODEC_ID_DSICINAUDIO:        os << "CODEC_ID_DSICINAUDIO";        break;
    case CODEC_ID_IMC:                os << "CODEC_ID_IMC";                break;
    case CODEC_ID_MUSEPACK7:          os << "CODEC_ID_MUSEPACK7";          break;
    case CODEC_ID_MLP:                os << "CODEC_ID_MLP";                break;
    case CODEC_ID_GSM_MS:             os << "CODEC_ID_GSM_MS";             break;
    case CODEC_ID_ATRAC3:             os << "CODEC_ID_ATRAC3";             break;
    case CODEC_ID_VOXWARE:            os << "CODEC_ID_VOXWARE";            break;
    case CODEC_ID_APE:                os << "CODEC_ID_APE";                break;
    case CODEC_ID_NELLYMOSER:         os << "CODEC_ID_NELLYMOSER";         break;
    case CODEC_ID_MUSEPACK8:          os << "CODEC_ID_MUSEPACK8";          break;
    case CODEC_ID_SPEEX:              os << "CODEC_ID_SPEEX";              break;
    case CODEC_ID_WMAVOICE:           os << "CODEC_ID_WMAVOICE";           break;
    case CODEC_ID_WMAPRO:             os << "CODEC_ID_WMAPRO";             break;
    case CODEC_ID_WMALOSSLESS:        os << "CODEC_ID_WMALOSSLESS";        break;
    case CODEC_ID_ATRAC3P:            os << "CODEC_ID_ATRAC3P";            break;
    case CODEC_ID_EAC3:               os << "CODEC_ID_EAC3";               break;
    case CODEC_ID_SIPR:               os << "CODEC_ID_SIPR";               break;
    case CODEC_ID_MP1:                os << "CODEC_ID_MP1";                break;
    case CODEC_ID_TWINVQ:             os << "CODEC_ID_TWINVQ";             break;
    case CODEC_ID_TRUEHD:             os << "CODEC_ID_TRUEHD";             break;
    case CODEC_ID_MP4ALS:             os << "CODEC_ID_MP4ALS";             break;
    case CODEC_ID_ATRAC1:             os << "CODEC_ID_ATRAC1";             break;
    case CODEC_ID_BINKAUDIO_RDFT:     os << "CODEC_ID_BINKAUDIO_RDFT";     break;
    case CODEC_ID_BINKAUDIO_DCT:      os << "CODEC_ID_BINKAUDIO_DCT";      break;
    case CODEC_ID_AAC_LATM:           os << "CODEC_ID_AAC_LATM";           break;
    case CODEC_ID_QDMC:               os << "CODEC_ID_QDMC";               break;
    case CODEC_ID_CELT:               os << "CODEC_ID_CELT";               break;
    case CODEC_ID_8SVX_EXP:           os << "CODEC_ID_8SVX_EXP";           break;
    case CODEC_ID_8SVX_FIB:           os << "CODEC_ID_8SVX_FIB";           break;
    case CODEC_ID_BMV_AUDIO:          os << "CODEC_ID_BMV_AUDIO";          break;
    case CODEC_ID_G729:               os << "CODEC_ID_G729";               break;
    case CODEC_ID_G723_1:             os << "CODEC_ID_G723_1";             break;
    case CODEC_ID_FFWAVESYNTH:        os << "CODEC_ID_FFWAVESYNTH";        break;
    case CODEC_ID_8SVX_RAW:           os << "CODEC_ID_8SVX_RAW";           break;
    case CODEC_ID_DVD_SUBTITLE:       os << "CODEC_ID_DVD_SUBTITLE";       break;
    case CODEC_ID_DVB_SUBTITLE:       os << "CODEC_ID_DVB_SUBTITLE";       break;
    case CODEC_ID_TEXT:               os << "CODEC_ID_TEXT";               break;
    case CODEC_ID_XSUB:               os << "CODEC_ID_XSUB";               break;
    case CODEC_ID_SSA:                os << "CODEC_ID_SSA";                break;
    case CODEC_ID_MOV_TEXT:           os << "CODEC_ID_MOV_TEXT";           break;
    case CODEC_ID_HDMV_PGS_SUBTITLE:  os << "CODEC_ID_HDMV_PGS_SUBTITLE";  break;
    case CODEC_ID_DVB_TELETEXT:       os << "CODEC_ID_DVB_TELETEXT";       break;
    case CODEC_ID_SRT:                os << "CODEC_ID_SRT";                break;
    case CODEC_ID_MICRODVD:           os << "CODEC_ID_MICRODVD";           break;
    case CODEC_ID_BINTEXT:            os << "CODEC_ID_BINTEXT";            break;
    case CODEC_ID_XBIN:               os << "CODEC_ID_XBIN";               break;
    case CODEC_ID_IDF:                os << "CODEC_ID_IDF";                break;
    case CODEC_ID_PROBE:              os << "CODEC_ID_PROBE";              break;
    case CODEC_ID_MPEG2TS:            os << "CODEC_ID_MPEG2TS";            break;
    case CODEC_ID_MPEG4SYSTEMS:       os << "CODEC_ID_MPEG4SYSTEMS";       break;
    case CODEC_ID_FFMETADATA:         os << "CODEC_ID_FFMETADATA";         break;
    default:                          os << static_cast<long>(obj) << "(Unknown CodecID)";
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const AVSampleFormat& obj)
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
    default:                 os << static_cast<long>(obj) << "(Unknown AVSampleFormat)";
    }
    return os;
};

std::string avcodecErrorString(int errorcode)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE];
    int errorDecodeResult = av_strerror(errorcode, buffer, AV_ERROR_MAX_STRING_SIZE);
    VAR_ERROR(errorDecodeResult);
    if (errorDecodeResult != 0)
    {
        return std::string("Avcodec error code not found");
    }
    return str( boost::format("'%1%'") % buffer );
}