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

#include "AudioTransition_CrossFade.h"

#include "AudioChunk.h"
#include "AudioClip.h"
#include "AudioCompositionParameters.h"
#include "Convert.h"
#include "TransitionFactory.h"
#include "UtilLog.h"

namespace model { namespace audio { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CrossFade::CrossFade()
    :	AudioTransition()
{
    VAR_DEBUG(this);
}

CrossFade::CrossFade(const CrossFade& other)
    :   AudioTransition(other)
{
    VAR_DEBUG(*this);
}

CrossFade* CrossFade::clone() const
{
    return new CrossFade(static_cast<const CrossFade&>(*this));
}

CrossFade::~CrossFade()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// AUDIOTRANSITION
//////////////////////////////////////////////////////////////////////////

void CrossFade::reset()
{
}

AudioChunkPtr CrossFade::getAudio(pts position, const IClipPtr& leftClip, const IClipPtr& rightClip, const AudioCompositionParameters& parameters)
{
    AudioChunkPtr leftChunk = 
        leftClip ? boost::static_pointer_cast<AudioClip>(leftClip)->getNextAudio(parameters)  : AudioChunkPtr();
    AudioChunkPtr rightChunk = 
        rightClip ? boost::static_pointer_cast<AudioClip>(rightClip)->getNextAudio(parameters) : AudioChunkPtr();
    ASSERT(!leftChunk || !rightChunk || leftChunk->getUnreadSampleCount() == rightChunk->getUnreadSampleCount());

    auto determineSampleCountAt = [parameters](pts position) -> samplecount
    {
        return Convert::ptsToSamples(parameters.getSampleRate(), parameters.getNrChannels(), position);
    };
        
    samplecount nSamples = parameters.getChunkSize();
    ASSERT_ZERO(nSamples % parameters.getNrChannels()); // Ensure that the data for all speakers is there... If this assert ever fails: maybe there's file formats in which the data for a frame is 'truncated'?
    sample* p = 0;
    model::AudioChunkPtr result = boost::make_shared<model::AudioChunk>(parameters.getNrChannels(), nSamples, true, false, p);

    ASSERT_LESS_THAN_EQUALS(getLeftPts() + position, getRightPts());
    samplecount leftSampleCount = determineSampleCountAt(getLeftPts());
    samplecount totalSampleCount = determineSampleCountAt(getRightPts()) - leftSampleCount; // Total number of samples in this transition, not just for this chunk
    samplecount currentSampleCount = determineSampleCountAt(getLeftPts() + position) - leftSampleCount;

    sample* dataLeft = leftChunk ? leftChunk->getUnreadSamples() : 0;
    sample* dataRight = rightChunk ? rightChunk->getUnreadSamples() : 0;
    sample* dataResult = result->getBuffer();
    for (samplecount i = 0; i < nSamples; ++i) // todo does not work properly for multiple channels? (should increment with 2 then?)
    {
        float factorLeft = ((float)totalSampleCount - (float)currentSampleCount) / (float)totalSampleCount;
        float factorRight = (float)currentSampleCount / (float)totalSampleCount;
        sample left = dataLeft ? dataLeft[i] : 0;
        sample right = dataRight ? dataRight[i] : 0;
        dataResult[i] = left * factorLeft + right * factorRight;
        currentSampleCount++;
    }

    VAR_DEBUG(*result);

    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CrossFade& obj)
{
    os << static_cast<const AudioTransition&>(obj) << '|';
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void CrossFade::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AudioTransition);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void CrossFade::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void CrossFade::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::audio::transition::CrossFade)