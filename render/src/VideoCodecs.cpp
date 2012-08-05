#include "VideoCodecs.h"

#include "UtilLog.h"
#include "VideoCodec.h"
#include "VideoCodecParameter.h"

namespace model { namespace render {

// static
VideoCodecMap VideoCodecs::sVideoCodecs;

// static
boost::bimap<int,wxString> VideoCodecs::mapToName;

// static
void VideoCodecs::add(wxString name, VideoCodec codec)
{
    sVideoCodecs[codec.getId()] = boost::make_shared<VideoCodec>(codec);
    typedef boost::bimap<int, wxString> bimap;
    mapToName.insert( bimap::value_type(codec.getId(), name) );
}

// static
void VideoCodecs::initialize()
{
    sVideoCodecs.clear();

    add(_("No video"),
        VideoCodec(CODEC_ID_NONE));

    add(_("MPEG 2 video"),
        VideoCodec(CODEC_ID_MPEG2VIDEO).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000)).
        addParameter(VideoCodecParameterBFrames().enable().setMinimum(0).setMaximum(100).setDefault(0))
        );

    add(_("MPEG 1 video"),
        VideoCodec(CODEC_ID_MPEG1VIDEO).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000)).
        addParameter(VideoCodecParameterMacroBlockDecision().enable().setDefault(FF_MB_DECISION_RD)) // Needed to avoid using macroblocks in which some coeffs overflow. This does not happen with normal video, it just happens here as the motion of the chroma plane does not match the luma plane. (this was part of the encoding example used for making the rendering class, now used to test the enum parameter handling...)
        );

    add(_("DV video"),
        VideoCodec(CODEC_ID_DVVIDEO).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("H264 video"),
        VideoCodec(CODEC_ID_H264).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("Motion JPEG"),
        VideoCodec(CODEC_ID_MJPEG).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("MPEG 4"),
        VideoCodec(CODEC_ID_MPEG4).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("MPEG 4 version 3"),
        VideoCodec(CODEC_ID_MSMPEG4V3).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("RAW video"),
        VideoCodec(CODEC_ID_RAWVIDEO).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("Theora"),
        VideoCodec(CODEC_ID_THEORA).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

    add(_("VP8"),
        VideoCodec(CODEC_ID_VP8).
        addParameter(VideoCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(4000000))
        );

}

// static
VideoCodecPtr VideoCodecs::getDefault()
{
    return find(CODEC_ID_NONE);
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