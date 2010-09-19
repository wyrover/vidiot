#include "AudioClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "AudioFile.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioClip::AudioClip()
    :	Clip()
{
    VAR_DEBUG(this);
}

AudioClip::AudioClip(AudioFilePtr file)
    :	Clip(file)
{
    VAR_DEBUG(this);
}

AudioClip::AudioClip(const AudioClip& other)
    :   Clip(other)
{
    VAR_DEBUG(this);
}

AudioClip* AudioClip::clone()
{ 
    return new AudioClip(static_cast<const AudioClip&>(*this)); 
}

AudioClip::~AudioClip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioClip::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk = boost::static_pointer_cast<AudioFile>(mRender)->getNextAudio(audioRate, nAudioChannels);
    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Clip>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
}
template void AudioClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
