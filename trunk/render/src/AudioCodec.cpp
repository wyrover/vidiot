#include "AudioCodec.h"

#include "AudioCodecParameter.h"
#include "Properties.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCodec::AudioCodec()
    :   mId(CODEC_ID_NONE)
    ,   mParameters()
{
}

AudioCodec::AudioCodec(CodecID id)
    :   mId(id)
    ,   mParameters()
{
}

AudioCodec::AudioCodec(const AudioCodec& other)
    :   mId(other.mId)
    ,   mParameters(make_cloned<ICodecParameter>(other.mParameters))
{
}

AudioCodec::~AudioCodec()
{
}

AudioCodec* AudioCodec::clone() const
{
    return new AudioCodec(static_cast<const AudioCodec&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool AudioCodec::operator== (const AudioCodec& other) const
{
    return (mId == other.mId) && equals(mParameters,other.mParameters);
}

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

CodecID AudioCodec::getId() const
{
    return mId;
}

AudioCodec& AudioCodec::addParameter(ICodecParameter& parameter)
{
    mParameters.push_back(make_cloned_ptr<ICodecParameter>(parameter));
    return *this;
}

ICodecParameters AudioCodec::getParameters()
{
    return mParameters;
}

AVStream* AudioCodec::addStream(AVFormatContext* context) const
{
    AVCodec* encoder = avcodec_find_encoder(mId);
    AVStream* stream = avformat_new_stream(context, encoder);
    ASSERT(stream);

    AVCodecContext* audio_codec = stream->codec;
    ASSERT_EQUALS(audio_codec->codec_type,AVMEDIA_TYPE_AUDIO);
    audio_codec->codec_id = mId;
    audio_codec->sample_fmt = AV_SAMPLE_FMT_S16;
    BOOST_FOREACH( ICodecParameterPtr parameter, mParameters )
    {
        parameter->set(audio_codec);
    }
    audio_codec->sample_rate = Properties::get()->getAudioFrameRate();
    audio_codec->channels = Properties::get()->getAudioNumberOfChannels();

    if (context->oformat->flags & AVFMT_GLOBALHEADER)
    {
        // Some formats want stream headers to be separate
        audio_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    return stream;
}

void AudioCodec::open(AVCodecContext* context) const
{
    AVCodec* codec = avcodec_find_encoder(context->codec_id);
    ASSERT(codec);
    int result = avcodec_open(context, codec);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioCodec& obj )
{
    os  << "AudioCodec:"
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
void AudioCodec::serialize(Archive & ar, const unsigned int version)
{
    ar & mId;
    ar & mParameters;
}
template void AudioCodec::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioCodec::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace