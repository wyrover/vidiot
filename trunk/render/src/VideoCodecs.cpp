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

    add(_("MPEG 2 video"),
        VideoCodec(CODEC_ID_MPEG2VIDEO).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterBFrames().setMinimum(0).setMaximum(100).setDefault(0)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );

    add(_("H264 video"),
        VideoCodec(CODEC_ID_H264).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );

    add(_("Motion JPEG"),
        VideoCodec(CODEC_ID_MJPEG).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );

    add(_("MPEG 4"),
        VideoCodec(CODEC_ID_MPEG4).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );

    add(_("MPEG 4 version 3"),
        VideoCodec(CODEC_ID_MSMPEG4V3).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );

    add(_("Theora"),
        VideoCodec(CODEC_ID_THEORA).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );

    add(_("VP8"),
        VideoCodec(CODEC_ID_VP8).
        addParameter(VideoCodecParameterBitrate().setMinimum(500).setMaximum(10000000).setDefault(4000000)).
        addParameter(VideoCodecParameterGopSize().setMinimum(0).setMaximum(100).setDefault(12))
        );
}

// static
VideoCodecPtr VideoCodecs::getDefault()
{
    return find(CODEC_ID_MPEG4);
}

// static
std::list<CodecID> VideoCodecs::all()
{
    std::list<CodecID> result;
    for ( VideoCodecMap::value_type entry : sVideoCodecs )
    {
        result.push_back(entry.first);
    }
    return result;
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