#include "VideoCodec.h"

#include "Properties.h"

#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilList.h"
#include "VideoCodecParameter.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCodec::VideoCodec()
    :   mId(CODEC_ID_NONE)
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
    BOOST_FOREACH( ICodecParameterPtr parameter, mParameters )
    {
        parameter->set(video_codec);
    }
    video_codec->width = Properties::get().getVideoSize().GetWidth(); // resolution must be a multiple of two
    video_codec->height = Properties::get().getVideoSize().GetHeight();

    // Fundamental unit of time (in seconds) in terms of which frame timestamps are represented.
    // For fixed-fps content, timebase should be 1/framerate and timestamp increments should be identically 1.
    video_codec->time_base.den = Properties::get().getFrameRate().denominator();
    video_codec->time_base.num = Properties::get().getFrameRate().numerator();
    video_codec->pix_fmt = PIX_FMT_YUV420P;
    if (context->oformat->flags & AVFMT_GLOBALHEADER)
    {
        // Some formats want stream headers to be separate
        video_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    return stream;
}

void VideoCodec::open(AVCodecContext* context) const
{
    AVCodec* codec = avcodec_find_encoder(context->codec_id);
    ASSERT(codec);
    int result = avcodec_open(context, codec);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoCodec& obj )
{
    os  << "VideoCodec:"
        << &obj    << '|'
        << obj.mId << '|';
    BOOST_FOREACH( ICodecParameterPtr parameter, obj.mParameters )
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
    ar & mId;
    ar & mParameters;
}
template void VideoCodec::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoCodec::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace