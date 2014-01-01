// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef MODEL_RENDER_VIDEO_CODEC_PARAMETER_H
#define MODEL_RENDER_VIDEO_CODEC_PARAMETER_H

#include "CodecParameter.h"

#include "UtilEnum.h"

namespace model { namespace render {

DECLAREENUM(VideoCodecParameterType,
    BitRate,
    BitRateTolerance,
    GopSize,
    BFrames,
    MacroBlockDecision);

wxString getHumanReadibleName(VideoCodecParameterType id);

// NOTE: If a parameter seems to be 'filled in' on the wrong AVCodecContext struct member, check for typos in the first parameter to the template instantiations below.

struct VideoCodecParameterBitrate
    :   public CodecParameterInt<VideoCodecParameterBitrate,VideoCodecParameterType,BitRate>
{
    void set(AVCodecContext* codec) override;
};

struct VideoCodecParameterBitrateTolerance
    :   public CodecParameterInt<VideoCodecParameterBitrateTolerance,VideoCodecParameterType,BitRateTolerance>
{
    void set(AVCodecContext* codec) override;
};

struct VideoCodecParameterBFrames
    :   public CodecParameterInt<VideoCodecParameterBFrames,VideoCodecParameterType,BFrames>
{
    void set(AVCodecContext* codec) override;
};

extern boost::bimap<int,wxString> MacroBlockDecisionEnumMapping;
struct VideoCodecParameterMacroBlockDecision
    :   public CodecParameterEnum<VideoCodecParameterMacroBlockDecision,VideoCodecParameterType,MacroBlockDecision,MacroBlockDecisionEnumMapping>
{
    void set(AVCodecContext* codec) override;
};

struct VideoCodecParameterGopSize
    :   public CodecParameterInt<VideoCodecParameterGopSize,VideoCodecParameterType,GopSize>
{
    void set(AVCodecContext* codec) override;
};

//        /**
//     * Motion estimation algorithm used for video coding.
//     * 1 (zero), 2 (full), 3 (log), 4 (phods), 5 (epzs), 6 (x1), 7 (hex),
//     * 8 (umh), 9 (iter), 10 (tesa) [7, 8, 10 are x264 specific, 9 is snow specific]
//     * - encoding: MUST be set by user.
//     * - decoding: unused
//     */
//    int me_method;
//
//    /**
//     * This is the fundamental unit of time (in seconds) in terms
//     * of which frame timestamps are represented. For fixed-fps content,
//     * timebase should be 1/framerate and timestamp increments should be
//     * identically 1.
//     * - encoding: MUST be set by user.
//     * - decoding: Set by libavcodec.
//     */
//    AVRational time_base;
//
//    /* - encoding parameters */
//    float qcompress;  ///< amount of qscale change between easy & hard scenes (0.0-1.0)
//    float qblur;      ///< amount of qscale smoothing over time (0.0-1.0)
//
//    /**
//     * minimum quantizer
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int qmin;
//
//    /**
//     * maximum quantizer
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int qmax;
//
//    /**
//     * maximum quantizer difference between frames
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int max_qdiff;
//
//    /**
//     * maximum number of B-frames between non-B-frames
//     * Note: The output will be delayed by max_b_frames+1 relative to the input.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int max_b_frames;
//
//    /**
//     * qscale factor between IP and B-frames
//     * If > 0 then the last P-frame quantizer will be used (q= lastp_q*factor+offset).
//     * If < 0 then normal ratecontrol will be done (q= -normal_q*factor+offset).
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float b_quant_factor;
//
//    /**
//     * luma single coefficient elimination threshold
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int luma_elim_threshold;
//
//    /**
//     * chroma single coeff elimination threshold
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int chroma_elim_threshold;
//
//    /**
//     * qscale offset between IP and B-frames
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float b_quant_offset;
//
//    /**
//     * 0-> h263 quant 1-> mpeg quant
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int mpeg_quant;
//
//      /**
//     * ratecontrol qmin qmax limiting method
//     * 0-> clipping, 1-> use a nice continous function to limit qscale wthin qmin/qmax.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float rc_qsquish;
//
//    /**
//     * maximum bitrate
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int rc_max_rate;
//
//    /**
//     * minimum bitrate
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int rc_min_rate;
//
//    /**
//     * qscale factor between P and I-frames
//     * If > 0 then the last p frame quantizer will be used (q= lastp_q*factor+offset).
//     * If < 0 then normal ratecontrol will be done (q= -normal_q*factor+offset).
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float i_quant_factor;
//
//    /**
//     * qscale offset between P and I-frames
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float i_quant_offset;
//
//    /**
//     * initial complexity for pass1 ratecontrol
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float rc_initial_cplx;
//
//    /**
//     * DCT algorithm, see FF_DCT_* below
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int dct_algo;
//    #define FF_DCT_AUTO    0
//#define FF_DCT_FASTINT 1
//#define FF_DCT_INT     2
//#define FF_DCT_MMX     3
//#define FF_DCT_MLIB    4
//#define FF_DCT_ALTIVEC 5
//#define FF_DCT_FAAN    6
//
//    /**
//     * luminance masking (0-> disabled)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float lumi_masking;
//
//    /**
//     * temporary complexity masking (0-> disabled)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float temporal_cplx_masking;
//
//    /**
//     * spatial complexity masking (0-> disabled)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float spatial_cplx_masking;
//
//    /**
//     * p block masking (0-> disabled)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float p_masking;
//
//    /**
//     * darkness masking (0-> disabled)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float dark_masking;
//
//    /**
//     * IDCT algorithm, see FF_IDCT_* below.
//     * - encoding: Set by user.
//     * - decoding: Set by user.
//     */
//        int idct_algo;
//        #define FF_IDCT_AUTO          0
//#define FF_IDCT_INT           1
//#define FF_IDCT_SIMPLE        2
//#define FF_IDCT_SIMPLEMMX     3
//#define FF_IDCT_LIBMPEG2MMX   4
//#define FF_IDCT_PS2           5
//#define FF_IDCT_MLIB          6
//#define FF_IDCT_ARM           7
//#define FF_IDCT_ALTIVEC       8
//#define FF_IDCT_SH4           9
//#define FF_IDCT_SIMPLEARM     10
//#define FF_IDCT_H264          11
//#define FF_IDCT_VP3           12
//#define FF_IDCT_IPP           13
//#define FF_IDCT_XVIDMMX       14
//#define FF_IDCT_CAVS          15
//#define FF_IDCT_SIMPLEARMV5TE 16
//#define FF_IDCT_SIMPLEARMV6   17
//#define FF_IDCT_SIMPLEVIS     18
//#define FF_IDCT_WMV2          19
//#define FF_IDCT_FAAN          20
//#define FF_IDCT_EA            21
//#define FF_IDCT_SIMPLENEON    22
//#define FF_IDCT_SIMPLEALPHA   23
//#define FF_IDCT_BINK          24
//
///**
//     * bits per sample/pixel from the demuxer (needed for huffyuv).
//     * - encoding: Set by libavcodec.
//     * - decoding: Set by user.
//     */
//     int bits_per_coded_sample;
//
//    /**
//     * prediction method (needed for huffyuv)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//     int prediction_method;
//#define FF_PRED_LEFT   0
//#define FF_PRED_PLANE  1
//#define FF_PRED_MEDIAN 2
//
//         /**
//     * sample aspect ratio (0 if unknown)
//     * That is the width of a pixel divided by the height of the pixel.
//     * Numerator and denominator must be relatively prime and smaller than 256 for some video standards.
//     * - encoding: Set by user.
//     * - decoding: Set by libavcodec.
//     */
//    AVRational sample_aspect_ratio;
//
//     /**
//     * motion estimation comparison function
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int me_cmp;
//    /**
//     * subpixel motion estimation comparison function
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int me_sub_cmp;
//    /**
//     * macroblock comparison function (not supported yet)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int mb_cmp;
//    /**
//     * interlaced DCT comparison function
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int ildct_cmp;
//#define FF_CMP_SAD    0
//#define FF_CMP_SSE    1
//#define FF_CMP_SATD   2
//#define FF_CMP_DCT    3
//#define FF_CMP_PSNR   4
//#define FF_CMP_BIT    5
//#define FF_CMP_RD     6
//#define FF_CMP_ZERO   7
//#define FF_CMP_VSAD   8
//#define FF_CMP_VSSE   9
//#define FF_CMP_NSSE   10
//#define FF_CMP_W53    11
//#define FF_CMP_W97    12
//#define FF_CMP_DCTMAX 13
//#define FF_CMP_DCT264 14
//#define FF_CMP_CHROMA 256
//
//    /**
//     * ME diamond size & shape
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int dia_size;
//
//    /**
//     * amount of previous MV predictors (2a+1 x 2a+1 square)
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int last_predictor_count;
//
//    /**
//     * prepass for motion estimation
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int pre_me;
//
//    /**
//     * motion estimation prepass comparison function
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int me_pre_cmp;
//
//    /**
//     * ME prepass diamond size & shape
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int pre_dia_size;
//
//    /**
//     * subpel ME quality
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int me_subpel_quality;
//
//    /**
//     * maximum motion estimation search range in subpel units
//     * If 0 then no limit.
//     *
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int me_range;
//
//    /**
//     * intra quantizer bias
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int intra_quant_bias;
//#define FF_DEFAULT_QUANT_BIAS 999999
//
//     /**
//     * inter quantizer bias
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int inter_quant_bias;
//
//    **
//     * Global quality for codecs which cannot change it per frame.
//     * This should be proportional to MPEG-1/2/4 qscale.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int global_quality;
//
//#define FF_CODER_TYPE_VLC       0
//#define FF_CODER_TYPE_AC        1
//#define FF_CODER_TYPE_RAW       2
//#define FF_CODER_TYPE_RLE       3
//#define FF_CODER_TYPE_DEFLATE   4
//    /**
//     * coder type
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int coder_type;
//
//    /**
//     * context model
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int context_model;
//
//    /**
//     * macroblock decision mode
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int mb_decision;
//#define FF_MB_DECISION_SIMPLE 0        ///< uses mb_cmp
//#define FF_MB_DECISION_BITS   1        ///< chooses the one which needs the fewest bits
//#define FF_MB_DECISION_RD     2        ///< rate distortion
//
//     /**
//     * scene change detection threshold
//     * 0 is default, larger means fewer detected scene changes.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int scenechange_threshold;
//
//    /**
//     * minimum Lagrange multipler
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int lmin;
//
//    /**
//     * maximum Lagrange multipler
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int lmax;
//
//     /**
//     * noise reduction strength
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int noise_reduction;
//
//    **
//     * Number of bits which should be loaded into the rc buffer before decoding starts.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int rc_initial_buffer_occupancy;
//
//    /**
//     *
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int inter_threshold;
//
//    /**
//     * CODEC_FLAG2_*
//     * - encoding: Set by user.
//     * - decoding: Set by user.
//     */
//    int flags2;
//
//    /**
//     * quantizer noise shaping
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int quantizer_noise_shaping;
//
//    /**
//     * Motion estimation threshold below which no motion estimation is
//     * performed, but instead the user specified motion vectors are used.
//     *
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//     int me_threshold;
//
//    /**
//     * Macroblock threshold below which the user specified macroblock types will be used.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//     int mb_threshold;
//
//    /**
//     * precision of the intra DC coefficient - 8
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//     int intra_dc_precision;
//
//    /**
//     * noise vs. sse weight for the nsse comparsion function
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//     int nsse_weight;
//
//     /**
//     * level
//     * - encoding: Set by user.
//     * - decoding: Set by libavcodec.
//     */
//     int level;
//#define FF_LEVEL_UNKNOWN -99
//
//    /**
//     * low resolution decoding, 1-> 1/2 size, 2->1/4 size
//     * - encoding: unused
//     * - decoding: Set by user.
//     */
//     int lowres;
//
//      /**
//     * frame skip threshold
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int frame_skip_threshold;
//
//    /**
//     * frame skip factor
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int frame_skip_factor;
//
//    /**
//     * frame skip exponent
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int frame_skip_exp;
//
//    /**
//     * frame skip comparison function
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int frame_skip_cmp;
//
//    /**
//     * Border processing masking, raises the quantizer for mbs on the borders
//     * of the picture.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    float border_masking;
//
//    /**
//     * minimum MB lagrange multipler
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int mb_lmin;
//
//    /**
//     * maximum MB lagrange multipler
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int mb_lmax;
//
//    /**
//     *
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int me_penalty_compensation;
//
//     /**
//     *
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int bidir_refine;
//
//    /**
//     *
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int brd_scale;
//
//    /**
//     * minimum GOP size
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int keyint_min;
//
//    /**
//     * number of reference frames
//     * - encoding: Set by user.
//     * - decoding: Set by lavc.
//     */
//    int refs;
//
//    /**
//     * chroma qp offset from luma
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int chromaoffset;
//
//     /**
//     * trellis RD quantization
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int trellis;
//
//    /**
//     * Multiplied by qscale for each frame and added to scene_change_score.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int scenechange_factor;
//
//    /**
//     *
//     * Note: Value depends upon the compare function used for fullpel ME.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int mv0_threshold;
//
//    /**
//     * Adjust sensitivity of b_frame_strategy 1.
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int b_sensitivity;
//
//    /**
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int compression_level;
//#define FF_COMPRESSION_DEFAULT -1
//
//     /**
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int min_prediction_order;
//
//    /**
//     * - encoding: Set by user.
//     * - decoding: unused
//     */
//    int max_prediction_order;
//
//    /**
//     * Bits per sample/pixel of internal libavcodec pixel/sample format.
//     * - encoding: set by user.
//     * - decoding: set by libavcodec.
//     */
//    int bits_per_raw_sample;
//    /**
//     * Ratecontrol attempt to use, at maximum, <value> of what can be used without an underflow.
//     * - encoding: Set by user.
//     * - decoding: unused.
//     */
//    float rc_max_available_vbv_use;
//
//    /**
//     * Ratecontrol attempt to use, at least, <value> times the amount needed to prevent a vbv overflow.
//     * - encoding: Set by user.
//     * - decoding: unused.
//     */
//    float rc_min_vbv_overflow_use;
// /**
//     * For some codecs, the time base is closer to the field rate than the frame rate.
//     * Most notably, H.264 and MPEG-2 specify time_base as half of frame duration
//     * if no telecine is used ...
//     *
//     * Set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2.
//     */
//    int ticks_per_frame;
//
//    /**
//     * Chromaticity coordinates of the source primaries.
//     * - encoding: Set by user
//     * - decoding: Set by libavcodec
//     */
//    enum AVColorPrimaries color_primaries;
//
//    /**
//     * Color Transfer Characteristic.
//     * - encoding: Set by user
//     * - decoding: Set by libavcodec
//     */
//    enum AVColorTransferCharacteristic color_trc;
//
//    /**
//     * YUV colorspace type.
//     * - encoding: Set by user
//     * - decoding: Set by libavcodec
//     */
//    enum AVColorSpace colorspace;
//
//    /**
//     * MPEG vs JPEG YUV range.
//     * - encoding: Set by user
//     * - decoding: Set by libavcodec
//     */
//    enum AVColorRange color_range;
//
//    /**
//     * This defines the location of chroma samples.
//     * - encoding: Set by user
//     * - decoding: Set by libavcodec
//     */
//    enum AVChromaLocation chroma_sample_location;

}} // namespace

BOOST_CLASS_VERSION(model::render::VideoCodecParameterBitrate, 1)
BOOST_CLASS_VERSION(model::render::VideoCodecParameterBFrames, 1)
BOOST_CLASS_VERSION(model::render::VideoCodecParameterGopSize, 1)
BOOST_CLASS_VERSION(model::render::VideoCodecParameterBitrateTolerance, 1)
BOOST_CLASS_VERSION(model::render::VideoCodecParameterMacroBlockDecision, 1)

BOOST_CLASS_EXPORT_KEY(model::render::VideoCodecParameterBitrate)
BOOST_CLASS_EXPORT_KEY(model::render::VideoCodecParameterBFrames)
BOOST_CLASS_EXPORT_KEY(model::render::VideoCodecParameterGopSize)
BOOST_CLASS_EXPORT_KEY(model::render::VideoCodecParameterBitrateTolerance)
BOOST_CLASS_EXPORT_KEY(model::render::VideoCodecParameterMacroBlockDecision)

#endif // MODEL_RENDER_VIDEO_CODEC_PARAMETER_H