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

#include "AudioClip.h"

#include "AudioClipEvent.h"
#include "AudioCompositionParameters.h"
#include "AudioFile.h"
#include "Constants.h"
#include "Convert.h"
#include "EmptyChunk.h"
#include "Node.h"
#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioClip::AudioClip()
    :	ClipInterval()
    ,   mProgress(0)
    ,   mVolume(Constants::sDefaultVolume)
{
    VAR_DEBUG(*this);
}

AudioClip::AudioClip(const AudioFilePtr& file)
    :	ClipInterval(file)
    ,   mProgress(0)
    ,   mVolume(Constants::sDefaultVolume)
{
    VAR_DEBUG(*this);
}

AudioClip::AudioClip(const AudioClip& other)
    :   ClipInterval(other)
    ,   mProgress(0)
    ,   mVolume(other.mVolume)
{
    VAR_DEBUG(*this)(other);
}

AudioClip* AudioClip::clone() const
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

void AudioClip::clean()
{
    VAR_DEBUG(this);
    mProgress = 0;
    ClipInterval::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

std::ostream& AudioClip::dump(std::ostream& os) const
{
    os << *this;
    return os;
}

const char* AudioClip::getType() const
{
    return "Audio";
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioClip::getNextAudio(const AudioCompositionParameters& parameters)
{
    if (getNewStartPosition())
    {
        mProgress = parameters.ptsToSamples(*getNewStartPosition()); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateNewStartPosition();
    }

    int lengthInSamples = parameters.ptsToSamples(getLength());

    AudioChunkPtr audioChunk;

    int remainingSamples = lengthInSamples - mProgress;

    if (remainingSamples > 0)
    {
        audioChunk = getDataGenerator<AudioFile>()->getNextAudio(parameters);
        if (audioChunk)
        {
            if (mProgress + audioChunk->getUnreadSampleCount() > lengthInSamples)
            {
                audioChunk->setAdjustedLength(lengthInSamples - mProgress);
                mProgress = lengthInSamples;
            }
            else
            {
                mProgress += audioChunk->getUnreadSampleCount();
            }
        }
        else
        {
            // The clip has not provided enough audio data yet (for the pts length of the clip)
            // but there is no more audio data. This can typically happen by using a avi file
            // for which the video data is longer than the audio data. Instead of clipping the
            // extra video part, silence is added here (the user can make the clip shorter if
            // required - thus removing the extra video, but that's a user decision to be made).
            LOG_WARNING << *this << ": (" << getDescription() << ") Adding " << remainingSamples << " samples to make audio length equal to video length";
            audioChunk = boost::static_pointer_cast<AudioChunk>(boost::make_shared<EmptyChunk>(parameters.getNrChannels(), remainingSamples));
            mProgress = lengthInSamples;
        }
    }
    VAR_DEBUG(*this)(mProgress)(lengthInSamples);

    if (audioChunk && !audioChunk->isA<EmptyChunk>() && mVolume != Constants::sDefaultVolume)
    {
        bool overflow = false;
        sample* sBegin = audioChunk->getUnreadSamples();
        sample* sEnd = sBegin + audioChunk->getUnreadSampleCount();
        sample* s = sBegin;
        int32_t volume = static_cast<int32_t>(mVolume);
        int32_t defaultVolume = static_cast<int32_t>(Constants::sDefaultVolume);
        while (s < sEnd)
        {
            //int32_t newSample = static_cast<int32_t>(*s) * volume / defaultVolume;
            //overflow = overflow || (newSample > std::numeric_limits<sample>::max());
            *s++ = static_cast<int32_t>(*s) * volume / defaultVolume; // newSample;
        }
    }

    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// AUDIOCLIP
//////////////////////////////////////////////////////////////////////////

void AudioClip::setVolume(int volume)
{
    if (volume != mVolume)
    {
        mVolume = volume;
        EventChangeAudioClipVolume event(volume);
        ProcessEvent(event);
    }
}

int AudioClip::getVolume() const
{
    return mVolume;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioClip& obj)
{
    os  << static_cast<const ClipInterval&>(obj) << '|'
        << std::setw(8) << obj.mProgress
        << std::setw(8) << obj.mVolume;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioClip::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ClipInterval);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
        if (version > 1)
        {
            ar & BOOST_SERIALIZATION_NVP(mVolume);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioClip)
