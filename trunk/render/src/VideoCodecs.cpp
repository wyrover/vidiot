#include "VideoCodecs.h"

#include <boost/make_shared.hpp>
#include "UtilLog.h"
#include "VideoCodec.h"
#include "VideoCodecParameter.h"

namespace model { namespace render {

// static
VideoCodecMap VideoCodecs::sVideoCodecs;

// static
void VideoCodecs::add(VideoCodec codec)
{
    sVideoCodecs[codec.getId()] = boost::make_shared<VideoCodec>(codec);
}

// static
void VideoCodecs::initialize()
{
    sVideoCodecs.clear();

    add(VideoCodec(CODEC_ID_A64_MULTI)
        .addParameter(VideoCodecParameterBitrate().enable().setDefault(10000).setMinimum(500).setMaximum(40000))
        .addParameter(VideoCodecParameterBFrames().enable().setDefault(2).setMinimum(0).setMaximum(100))
        );

    add(VideoCodec(CODEC_ID_MPEG2VIDEO).
        addParameter(VideoCodecParameterBitrate().enable().setDefault(10000).setMinimum(500).setMaximum(40000))
        );

    add(VideoCodec(CODEC_ID_MPEG1VIDEO).
        addParameter(VideoCodecParameterBitrate().enable().setDefault(10000).setMinimum(500).setMaximum(40000))
        );

    //if (video_codec->codec_id == CODEC_ID_MPEG2VIDEO)
    //{
    //    video_codec->max_b_frames = 2; // just for testing, we also add B frames
    //}
    //if (video_codec->codec_id == CODEC_ID_MPEG1VIDEO)
    //{
    //    // Needed to avoid using macroblocks in which some coeffs overflow.
    //    // This does not happen with normal video, it just happens here as
    //    // the motion of the chroma plane does not match the luma plane.
    //    video_codec->mb_decision=2;
    //}

}

// static
VideoCodecPtr VideoCodecs::find(CodecID id)
{
    VideoCodecMap::const_iterator it = sVideoCodecs.find(id);
    if (it == sVideoCodecs.end())
    {
        return VideoCodecPtr();
    }
    return make_cloned<VideoCodec>(it->second);
}

}} //namespace