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

#include "VideoCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(VideoCodecParameterType);

// Check mpegvideo_enc.c for all parameters and their bounds and dependencies...

boost::bimap<VideoCodecParameterType, wxString> VideoCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<VideoCodecParameterType, wxString>::relation >
    (BitRate, _("Bit rate"))
    (BitRateTolerance, _("Bit rate tolerance"))
    (GopSize, _("Gop size (max frame distance between 2 I-frames)"))
    (BFrames, _("B Frames"))
    (MacroBlockDecision, _("Macro block decision"));

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
    codec->bit_rate_tolerance = getValue(); // Must ensure that avctx->bit_rate * av_q2d(avctx->time_base) > avctx->bit_rate_tolerance)
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

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::VideoCodecParameterBitrate)
BOOST_CLASS_EXPORT_IMPLEMENT(model::render::VideoCodecParameterBFrames)
BOOST_CLASS_EXPORT_IMPLEMENT(model::render::VideoCodecParameterGopSize)
BOOST_CLASS_EXPORT_IMPLEMENT(model::render::VideoCodecParameterBitrateTolerance)
BOOST_CLASS_EXPORT_IMPLEMENT(model::render::VideoCodecParameterMacroBlockDecision)