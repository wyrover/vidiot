#include "VideoCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(VideoCodecParameterType);

boost::bimap<VideoCodecParameterType, wxString> VideoCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<VideoCodecParameterType, wxString>::relation >
    (BitRate, _("Bit rate"))
    (GopSize, _("Gop size"))
    (BFrames, _("B Frames"))
    (MacroBlockDecision, _("Macro block decision"));

wxString getHumanReadibleName(VideoCodecParameterType id)
{
    return (VideoCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

void VideoCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

void VideoCodecParameterBitrateTolerance::set(AVCodecContext* codec)
{
    codec->bit_rate_tolerance = getValue();
};

void VideoCodecParameterBFrames::set(AVCodecContext* codec)
{
    codec->max_b_frames = getValue();
};

boost::bimap<int,wxString> MacroBlockDecisionEnumMapping = boost::assign::list_of<boost::bimap<int, wxString>::relation >
    //(FF_MB_DECISION_SIMPLE, _("Simple - use mb_cmp (Not supported yet)"))
    (FF_MB_DECISION_BITS,   _("Least bits"))
    (FF_MB_DECISION_RD,     _("Rate distortion"));

void VideoCodecParameterMacroBlockDecision::set(AVCodecContext* codec)
{
    codec->mb_decision = getValue();
};

// todo register types for all of the params.

}} //namespace