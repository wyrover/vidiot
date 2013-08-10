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

#ifndef RENDER_VIDEO_CODEC_H
#define RENDER_VIDEO_CODEC_H

#include "UtilCloneable.h"

namespace model { namespace render {

class VideoCodec
    :   public Cloneable<VideoCodec>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoCodec();

    explicit VideoCodec(CodecID id);
    VideoCodec(const VideoCodec& other);
    virtual ~VideoCodec();

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const VideoCodec& other) const;

    //////////////////////////////////////////////////////////////////////////
    // PARAMETERS
    //////////////////////////////////////////////////////////////////////////

    CodecID getId() const;
    VideoCodec& addParameter(ICodecParameter& parameter);
    ICodecParameters getParameters();
    AVStream* addStream(AVFormatContext* context) const; ///< Add a stream to the given libavformat format
    bool open(AVCodecContext* context) const;            ///< Open the codec in libavcodec

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    CodecID mId;
    ICodecParameters mParameters;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoCodec& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

BOOST_CLASS_VERSION(model::render::VideoCodec, 1)

#endif // RENDER_VIDEO_CODEC_H