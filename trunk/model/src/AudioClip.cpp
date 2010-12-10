#include "AudioClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "AudioFile.h"
#include "Convert.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioClip::AudioClip()
    :	Clip()
    ,   mProgress(boost::none)
    ,   mLastSetPosition(0)
{
    VAR_DEBUG(this);
}

AudioClip::AudioClip(AudioFilePtr file)
    :	Clip(file)
    ,   mProgress(boost::none)
    ,   mLastSetPosition(0)
{
    VAR_DEBUG(this);
}

AudioClip::AudioClip(const AudioClip& other)
    :   Clip(other)
    ,   mProgress(boost::none)
    ,   mLastSetPosition(0)
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
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void AudioClip::moveTo(pts position)
{
    mProgress.reset();
    mLastSetPosition = position;
    Clip::moveTo(position);
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioClip::getNextAudio(int audioRate, int nAudioChannels)
{
    unsigned int progress = 0;
    if (!mProgress)
    {
        // Initialize mProgress to the last value set in ::moveTo
        progress = 
            audioRate *
            nAudioChannels * 
            model::Convert::ptsToTime(mLastSetPosition) /
            1000; // ms/s
    }
    else
    {
        progress = *mProgress;
    }

    unsigned int length = 
        audioRate *
        nAudioChannels * 
        model::Convert::ptsToTime(getNumberOfFrames()) / // ms
        1000; // ms/s

    AudioChunkPtr audioChunk;

    if (progress < length)
    {
        audioChunk = getDataGenerator<AudioFile>()->getNextAudio(audioRate, nAudioChannels);
        if (audioChunk)
        {
            if (progress + audioChunk->getUnreadSampleCount() > length) // todo make Convert::samplesToBytes
            {
                audioChunk->setAdjustedLength(length - progress);
                mProgress.reset(length);
            }
            else
            {
                mProgress.reset(audioChunk->getUnreadSampleCount());
            }
        }
        else
        {
            // Todo: Clip is longer than original data
        }
    }

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
