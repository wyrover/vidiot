// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef MODEL_RENDER_AUDIO_CODEC_PARAMETER_H
#define MODEL_RENDER_AUDIO_CODEC_PARAMETER_H

#include "CodecParameter.h"
#include "UtilEnum.h"

namespace model { namespace render {

DECLAREENUM(AudioCodecParameterType, \
    AudioBitRate,
    AudioUnused); // GCC macro requires more than 1 enum value

template < class Archive >
void registerAudioCodecParameterTypesForSerializaton(Archive& ar);

wxString getHumanReadibleName(const AudioCodecParameterType& id);
AudioCodecParameterType getAudioCodecIdFromHumanReadibleName(const wxString& name);

// NOTE: If a parameter seems to be 'filled in' on the wrong AVCodecContext struct member, check for typos in the first parameter to the template instantiations below.

struct AudioCodecParameterBitrate
    :   public CodecParameterInt<AudioCodecParameterBitrate,AudioCodecParameterType,AudioBitRate>
{
    void set(AVCodecContext* codec) override;
};

//    /* audio only */
//    int sample_rate; ///< samples per second
//    int channels;    ///< number of audio channels
//
//     /**
//     * audio sample format
//     * - encoding: Set by user.
//     * - decoding: Set by libavcodec.
//     */
//    enum AVSampleFormat sample_fmt;  ///< sample format
//
//
//     /**
//     * profile
//     * - encoding: Set by user.
//     * - decoding: Set by libavcodec.
//     */
//     int profile;
//#define FF_PROFILE_UNKNOWN -99
//#define FF_PROFILE_RESERVED -100
//
//#define FF_PROFILE_AAC_MAIN 0
//#define FF_PROFILE_AAC_LOW  1
//#define FF_PROFILE_AAC_SSR  2
//#define FF_PROFILE_AAC_LTP  3
//
//#define FF_PROFILE_DTS         20
//#define FF_PROFILE_DTS_ES      30
//#define FF_PROFILE_DTS_96_24   40
//#define FF_PROFILE_DTS_HD_HRA  50
//#define FF_PROFILE_DTS_HD_MA   60
//
//#define FF_PROFILE_MPEG2_422    0
//#define FF_PROFILE_MPEG2_HIGH   1
//#define FF_PROFILE_MPEG2_SS     2
//#define FF_PROFILE_MPEG2_SNR_SCALABLE  3
//#define FF_PROFILE_MPEG2_MAIN   4
//#define FF_PROFILE_MPEG2_SIMPLE 5
//
//#define FF_PROFILE_H264_CONSTRAINED  (1<<9)  // 8+1; constraint_set1_flag
//#define FF_PROFILE_H264_INTRA        (1<<11) // 8+3; constraint_set3_flag
//
//#define FF_PROFILE_H264_BASELINE             66
//#define FF_PROFILE_H264_CONSTRAINED_BASELINE (66|FF_PROFILE_H264_CONSTRAINED)
//#define FF_PROFILE_H264_MAIN                 77
//#define FF_PROFILE_H264_EXTENDED             88
//#define FF_PROFILE_H264_HIGH                 100
//#define FF_PROFILE_H264_HIGH_10              110
//#define FF_PROFILE_H264_HIGH_10_INTRA        (110|FF_PROFILE_H264_INTRA)
//#define FF_PROFILE_H264_HIGH_422             122
//#define FF_PROFILE_H264_HIGH_422_INTRA       (122|FF_PROFILE_H264_INTRA)
//#define FF_PROFILE_H264_HIGH_444             144
//#define FF_PROFILE_H264_HIGH_444_PREDICTIVE  244
//#define FF_PROFILE_H264_HIGH_444_INTRA       (244|FF_PROFILE_H264_INTRA)
//#define FF_PROFILE_H264_CAVLC_444            44
//
//#define FF_PROFILE_VC1_SIMPLE   0
//#define FF_PROFILE_VC1_MAIN     1
//#define FF_PROFILE_VC1_COMPLEX  2
//#define FF_PROFILE_VC1_ADVANCED 3
//
//#define FF_PROFILE_MPEG4_SIMPLE                     0
//#define FF_PROFILE_MPEG4_SIMPLE_SCALABLE            1
//#define FF_PROFILE_MPEG4_CORE                       2
//#define FF_PROFILE_MPEG4_MAIN                       3
//#define FF_PROFILE_MPEG4_N_BIT                      4
//#define FF_PROFILE_MPEG4_SCALABLE_TEXTURE           5
//#define FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION      6
//#define FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE     7
//#define FF_PROFILE_MPEG4_HYBRID                     8
//#define FF_PROFILE_MPEG4_ADVANCED_REAL_TIME         9
//#define FF_PROFILE_MPEG4_CORE_SCALABLE             10
//#define FF_PROFILE_MPEG4_ADVANCED_CODING           11
//#define FF_PROFILE_MPEG4_ADVANCED_CORE             12
//#define FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE 13
//#define FF_PROFILE_MPEG4_SIMPLE_STUDIO             14
//#define FF_PROFILE_MPEG4_ADVANCED_SIMPLE           15
//
//     /**
//     * Audio cutoff bandwidth (0 means "automatic")
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int cutoff;
//
//    /**
//     * Audio channel layout.
//     * - encoding: set by user.
//     * - decoding: set by user, may be overwritten by libavcodec.
//     */
//    uint64_t channel_layout;
//
//    /**
//     * Request decoder to use this channel layout if it can (0 for default)
//     * - encoding: unused
//     * - decoding: Set by user.
//     */
//    uint64_t request_channel_layout;
//

}} // namespace

BOOST_CLASS_VERSION(model::render::AudioCodecParameterBitrate, 1)
BOOST_CLASS_EXPORT_KEY(model::render::AudioCodecParameterBitrate)

#endif
