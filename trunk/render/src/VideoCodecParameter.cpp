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

#include "VideoCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(VideoCodecParameterType);

// Check mpegvideo_enc.c for all parameters and their bounds and dependencies...

std::map<VideoCodecParameterType, wxString> VideoCodecParameterTypeConverter::getMapToHumanReadibleString()
{
    return
    {
        { BitRate, "Bit rate" },
        { BitRateTolerance, "Bit rate tolerance" },
        { GopSize, "Gop size" },
        { BFrames, "B Frames" },
        { MacroBlockDecision, "Macro block decision" },
    };
}

wxString getHumanReadibleName(const VideoCodecParameterType& id)
{
    std::map<VideoCodecParameterType, wxString> mapToHumanReadibleString = VideoCodecParameterTypeConverter::getMapToHumanReadibleString();
    ASSERT(mapToHumanReadibleString.find(id) != mapToHumanReadibleString.end())(id);
    return (mapToHumanReadibleString.find(id))->second;
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

std::map<int, wxString> MacroBlockDecisionEnumMapping = {
    //(FF_MB_DECISION_SIMPLE, _("Simple - use mb_cmp (Not supported yet)"))
    { FF_MB_DECISION_BITS, "Least bits" },
    { FF_MB_DECISION_RD, "Rate distortion" }
};

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