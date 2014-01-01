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

#include "AudioCodec.h"

#include "AudioCodecParameter.h"
#include "AudioCodecs.h"
#include "Dialog.h"
#include "Properties.h"
#include "UtilClone.h"
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

AudioCodec* AudioCodec::clone() const
{
    return new AudioCodec(static_cast<const AudioCodec&>(*this));
}

void AudioCodec::onCloned()
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
    for ( ICodecParameterPtr parameter : mParameters )
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
    int result = avcodec_open2(context, codec, 0);
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
void AudioCodec::serialize(Archive & ar, const unsigned int version)
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
template void AudioCodec::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioCodec::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::AudioCodec)