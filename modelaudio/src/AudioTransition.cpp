// Copyright 2013-2016 Eric Raijmakers.
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
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void AudioTransition::clean()
{
    VAR_DEBUG(this);
    Transition::clean();
    mLeftClip.reset();
    mRightClip.reset();
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

 AudioChunkPtr AudioTransition::getNextAudio(const AudioCompositionParameters& parameters)
{
    if (getNewStartPosition())
    {
        mProgress = *getNewStartPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateNewStartPosition();

        // Note: When creating a transition, the left and right clip are adjusted (shortened) to
        // accomodate for the addition of the transition. Therefore, the computations below take these
        // shortened clips as input.

        if (getLeft())
        {
            ASSERT(getPrev());
            mLeftClip = makeLeftClip();
            mLeftClip->moveTo(mProgress);
        }
        if (getRight()) 
        {
            ASSERT(getNext());
            mRightClip = makeRightClip();
            mRightClip->moveTo(mProgress);
        }

        ASSERT(!mLeftClip || !mRightClip || mLeftClip->getLength() == mRightClip->getLength());
    }

    AudioChunkPtr audioChunk;

    if (mProgress < getLength())
    {
        audioChunk = getAudio(mProgress, mLeftClip, mRightClip, parameters);
        mProgress++;
    }
    return audioChunk;
}

 //////////////////////////////////////////////////////////////////////////
 // LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioTransition& obj)
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
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Transition);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioTransition::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioTransition::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioTransition)