#include "VideoCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(VideoCodecParameterType);

// todo check mpegvideo_enc.c for all parameters and their bounds and dependencies...

boost::bimap<VideoCodecParameterType, wxString> VideoCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<VideoCodecParameterType, wxString>::relation >
    (BitRate, _("Bit rate"))
    (BitRateTolerance, _("Bit rate tolerance"))
    (GopSize, _("Gop size (max frame distance between 2 I-frames)"))
    (BFrames, _("B Frames"))
    (MacroBlockDecision, _("Macro block decision"));

template < class Archive >
void registerVideoCodecParameterTypesForSerializaton(Archive& ar)
{
    ar.template register_type<render::VideoCodecParameterBFrames>();
    ar.template register_type<render::VideoCodecParameterBitrate>();
    ar.template register_type<render::VideoCodecParameterBitrateTolerance>();
    ar.template register_type<render::VideoCodecParameterMacroBlockDecision>();
    ar.template register_type<render::VideoCodecParameterGopSize>();
}

template void registerVideoCodecParameterTypesForSerializaton<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void registerVideoCodecParameterTypesForSerializaton<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

wxString getHumanReadibleName(VideoCodecParameterType id)
{
    ASSERT(VideoCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id) != VideoCodecParameterTypeConverter::mapToHumanReadibleString.left.end())(id);
    return (VideoCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

void VideoCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

void VideoCodecParameterBitrateTolerance::set(AVCodecContext* codec)
{
    codec->bit_rate_tolerance = getValue();// todo ensure that avctx->bit_rate * av_q2d(avctx->time_base) >         avctx->bit_rate_tolerance) {
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

void VideoCodecParameterGopSize::set(AVCodecContext* codec)
{
    codec->gop_size = getValue();
};

}} //namespace