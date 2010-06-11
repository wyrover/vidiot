#include "AudioTrack.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "AudioClip.h"

namespace model {

AudioTrack::AudioTrack()
:	Track()
{ 
    VAR_DEBUG(this);

    mItClips = mClips.begin();
}

AudioTrack::~AudioTrack()
{
    VAR_DEBUG(this);
}

void AudioTrack::addAudioClip(AudioClipPtr clip)
{
    mClips.push_back(clip);
}

void AudioTrack::removeAudioClip(AudioClipPtr clip)
{
    NIY
}

AudioChunkPtr AudioTrack::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk = AudioChunkPtr();

    while (!audioChunk && mItClips != mClips.end())
    {
        audioChunk = boost::static_pointer_cast<AudioClip>(*mItClips)->getNextAudio(audioRate, nAudioChannels);
        if (!audioChunk)
        {
            mItClips++;
            if (mItClips != mClips.end())
            {
                (*mItClips)->moveTo(0);
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
