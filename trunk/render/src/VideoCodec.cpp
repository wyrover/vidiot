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

#include "VideoCodec.h"

#include "Dialog.h"
#include "Properties.h"
#include "UtilInitAvcodec.h"
#include "UtilCloneable.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogStl.h"
#include "VideoCodecs.h"
#include "VideoCodecParameter.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCodec::VideoCodec()
    :   mId(VideoCodecs::getDefault()->getId())
    ,   mParameters()
{
}

VideoCodec::VideoCodec(CodecID id)
    :   mId(id)
    ,   mParameters()
{
}

VideoCodec::VideoCodec(const VideoCodec& other)
    :   mId(other.mId)
    ,   mParameters(make_cloned<ICodecParameter>(other.mParameters))
{
}

VideoCodec* VideoCodec::clone() const
{
    return new VideoCodec(static_cast<const VideoCodec&>(*this));
}

void VideoCodec::onCloned()
{
}

VideoCodec::~VideoCodec()
{
}

//////////////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool VideoCodec::operator== (const VideoCodec& other) const
{
    return (mId == other.mId) && equals(mParameters,other.mParameters);
}

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

CodecID VideoCodec::getId() const
{
    return mId;
}

VideoCodec& VideoCodec::addParameter(ICodecParameter& parameter)
{
    ICodecParameter* clone = static_cast<ICodecParameter*>(parameter.clone());
    ASSERT(clone);
    ICodecParameterPtr newParam = boost::shared_ptr<ICodecParameter>(clone);
    ASSERT(newParam);
    mParameters.push_back(newParam);
    return *this;
}

ICodecParameters VideoCodec::getParameters()
{
    return mParameters;
}

AVStream* VideoCodec::addStream(AVFormatContext* context) const
{
    AVCodec* encoder = avcodec_find_encoder(mId);
    AVStream* stream = avformat_new_stream(context, encoder);
    ASSERT(stream);

    AVCodecContext* video_codec = stream->codec;
    ASSERT_EQUALS(video_codec->codec_type,AVMEDIA_TYPE_VIDEO);
    video_codec->codec_id = mId;
    for ( ICodecParameterPtr parameter : mParameters )
    {
        parameter->set(video_codec);
    }
    video_codec->width = Properties::get().getVideoSize().GetWidth(); // resolution must be a multiple of two
    video_codec->height = Properties::get().getVideoSize().GetHeight();

    // Fundamental unit of time (in seconds) in terms of which frame timestamps are represented.
    // For fixed-fps content, timebase should be 1/framerate and timestamp increments should be identically 1.
    video_codec->time_base.den = Properties::get().getFrameRate().numerator();
    video_codec->time_base.num = Properties::get().getFrameRate().denominator();

    ASSERT(encoder->pix_fmts);
    const PixelFormat* f = encoder->pix_fmts;
    VAR_DEBUG(*f);
    video_codec->pix_fmt = *f; // Use first pixel format

    if (context->oformat->flags & AVFMT_GLOBALHEADER)
    {
        // Some formats want stream headers to be separate
        video_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    return stream;
}

bool VideoCodec::open(AVCodecContext* context) const
{
    AVCodec* codec = avcodec_find_encoder(context->codec_id);
    ASSERT(codec);
    boost::mutex::scoped_lock lock(Avcodec::sMutex);
    int result = avcodec_open2(context, codec, 0);

    if (result < 0)
    {
        VAR_ERROR(codec)(context);
        // Now do the checks that ffmpeg does when opening the codec to give proper feedback
        gui::Dialog::get().getConfirmation( _("Error in video codec"), _("There was an error when opening the video codec.\nRendering will be aborted.\nDetailed information:\n") + Avcodec::getMostRecentLogLine());
    }
    return result >= 0;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoCodec& obj )
{
    os  << "VideoCodec:"
        << &obj    << '|'
        << obj.mId << '|';
    for ( ICodecParameterPtr parameter : obj.mParameters )
    {
        os << *parameter;
    }
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoCodec::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mId);
        ar & BOOST_SERIALIZATION_NVP(mParameters);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoCodec::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoCodec::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::VideoCodec)