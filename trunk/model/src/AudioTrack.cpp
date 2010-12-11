#include "AudioTrack.h"

#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "AudioClip.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioTrack::AudioTrack()
:	Track()
{ 
    VAR_DEBUG(this);
}

AudioTrack::AudioTrack(const AudioTrack& other)
:	Track(other)
{
    VAR_DEBUG(this);
}

AudioTrack* AudioTrack::clone()
{ 
    return new AudioTrack(static_cast<const AudioTrack&>(*this)); 
}

AudioTrack::~AudioTrack()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// PLAYBACK
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioTrack::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk;

    while (!audioChunk && iterate_hasClip())
    {
        audioChunk = boost::dynamic_pointer_cast<IAudio>(iterate_getClip())->getNextAudio(audioRate, nAudioChannels);
        if (!audioChunk)
        {
            iterate_nextClip();
            if (iterate_hasClip())
            {
                iterate_getClip()->moveTo(0);
            }
        }
    }
    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioTrack::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Track>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
}
template void AudioTrack::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioTrack::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
