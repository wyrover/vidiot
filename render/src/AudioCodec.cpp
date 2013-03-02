#include "AudioCodec.h"

#include "AudioCodecParameter.h"
#include "AudioCodecs.h"
#include "Dialog.h"
#include "Properties.h"
#include "UtilInitAvcodec.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCodec::AudioCodec()
    :   mId(AudioCodecs::getDefault()->getId())
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
    ICodecParameter* clone = static_cast<ICodecParameter*>(parameter.clone());
    ASSERT(clone);
    ICodecParameterPtr newParam = boost::shared_ptr<ICodecParameter>(clone);
    ASSERT(newParam);
    mParameters.push_back(newParam);
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
    audio_codec->sample_rate = Properties::get().getAudioFrameRate();
    audio_codec->channels = Properties::get().getAudioNumberOfChannels();

    if (context->oformat->flags & AVFMT_GLOBALHEADER)
    {
        // Some formats want stream headers to be separate
        audio_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    return stream;
}

bool AudioCodec::open(AVCodecContext* context) const
{
    AVCodec* codec = avcodec_find_encoder(context->codec_id);
    ASSERT(codec);
    boost::mutex::scoped_lock lock(Avcodec::sMutex);
    int result = avcodec_open(context, codec); // todo use open2
    if (result < 0)
    {
        // Now do the checks that ffmpeg does when opening the codec to give proper feedback
        gui::Dialog::get().getConfirmation( _("Error in audio codec"), _("There was an error when opening the audio codec.\nRendering will be aborted.\nDetailed information:\n") + Avcodec::getMostRecentLogLine());
    }
    return result >= 0;
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