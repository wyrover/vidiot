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

#include "AudioTransition.h"

#include "UtilInt.h"
#include "UtilLog.h"
#include "AudioClip.h"
#include "AudioChunk.h"
#include "Convert.h"
#include "AudioCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioTransition::AudioTransition()
    :	Transition()
    ,   IAudio()
    ,   mProgress(-1)
    ,   mLeftClip()
    ,   mRightClip()
{
    VAR_DEBUG(this);
}

AudioTransition::AudioTransition(const AudioTransition& other)
    :   Transition(other)
    ,   IAudio()
    ,   mProgress(-1)
    ,   mLeftClip()
    ,   mRightClip()
{
    VAR_DEBUG(*this);
}

AudioTransition::~AudioTransition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

 AudioChunkPtr AudioTransition::getNextAudio(const AudioCompositionParameters& parameters)
{
    if (getNewStartPosition())
    {
        pts ptsProgress = *getNewStartPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateNewStartPosition();

        // Note: When creating a transition, the left and right clip are adjusted (shortened) to
        // accomodate for the addition of the transition. Therefore, the computations below take these
        // shortened clips as input.

        if (getLeft() > 0)
        {
            ASSERT(getPrev());
            mLeftClip = makeLeftClip();
            mLeftClip->moveTo(ptsProgress);
        }
        if (getRight() > 0)
        {
            ASSERT(getNext());
            mRightClip = makeRightClip();
            mRightClip->moveTo(ptsProgress);
        }

        mProgress = parameters.ptsToSamples(ptsProgress);
        reset();

        ASSERT(!mLeftClip || !mRightClip || mLeftClip->getLength() == mRightClip->getLength());
    }
    AudioChunkPtr chunk;
    if (mProgress < getTotalSamples(parameters))
    {
        chunk = getAudio(mProgress, mLeftClip, mRightClip, parameters);
        chunk->setPts(Convert::samplesToPts(parameters.getSampleRate(), parameters.getNrChannels(), mProgress));
        mProgress += chunk->getUnreadSampleCount();
    }
    VAR_AUDIO(chunk);
    setGenerationProgress(mProgress);
    return chunk;
}

 //////////////////////////////////////////////////////////////////////////
 // IMPLEMENTATION OF TRANSITION
 //////////////////////////////////////////////////////////////////////////

samplecount AudioTransition::getTotalSamples(const AudioCompositionParameters& parameters) const
 {
     return parameters.ptsToSamples(getLength());

 }

 //////////////////////////////////////////////////////////////////////////
 // LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioTransition& obj )
{
    os << static_cast<const Transition&>(obj) << '|' << obj.mProgress;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioTransition::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & boost::serialization::base_object<Transition>(*this);
        ar & boost::serialization::base_object<IAudio>(*this);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioTransition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioTransition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace