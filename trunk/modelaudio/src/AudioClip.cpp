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

#include "AudioClip.h"

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
{
    VAR_DEBUG(*this);
}

AudioClip::AudioClip(AudioFilePtr file)
    :	ClipInterval(file)
    ,   mProgress(0)
{
    VAR_DEBUG(*this);
}

AudioClip::AudioClip(const AudioClip& other)
    :   ClipInterval(other)
    ,   mProgress(0)
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

char* AudioClip::getType() const
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

    VAR_AUDIO(audioChunk);
    setGenerationProgress(parameters.samplesToPts(mProgress));
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioClip& obj )
{
    os << static_cast<const ClipInterval&>(obj) << '|' << std::setw(8) << obj.mProgress;
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
        ar & boost::serialization::base_object<ClipInterval>(*this);
        ar & boost::serialization::base_object<IAudio>(*this);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace