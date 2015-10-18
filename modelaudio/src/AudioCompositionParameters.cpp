// Copyright 2013-2015 Eric Raijmakers.
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

#include "AudioCompositionParameters.h"

#include "Config.h"
#include "Convert.h"
#include "Properties.h"
#include "UtilLogBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters::AudioCompositionParameters()
    : mSampleRate(Properties::get().getAudioSampleRate())
    , mNrChannels(Properties::get().getAudioNumberOfChannels())
    , mSpeed(1)
    , mPts(boost::none)
    , mChunkSize(boost::none)
{
}

AudioCompositionParameters::AudioCompositionParameters(const AudioCompositionParameters& other)
    : mSampleRate(other.mSampleRate)
    , mNrChannels(other.mNrChannels)
    , mSpeed(other.mSpeed)
    , mPts(other.mPts)
    , mChunkSize(other.mChunkSize)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters& AudioCompositionParameters::setSampleRate(int audiorate)
{
    mSampleRate = audiorate;
    return *this;
}

int AudioCompositionParameters::getSampleRate() const
{
    return mSampleRate;
}

AudioCompositionParameters& AudioCompositionParameters::setNrChannels(int nChannels)
{
    mNrChannels = nChannels;
    return *this;
}

int AudioCompositionParameters::getNrChannels() const
{
    return mNrChannels;
}

AudioCompositionParameters& AudioCompositionParameters::setSpeed(boost::rational<int> speed)
{
    mSpeed = speed;
    return *this;
}
 
boost::rational<int> AudioCompositionParameters::getSpeed() const
{
    return mSpeed;
}

AudioCompositionParameters& AudioCompositionParameters::setPts(pts position)
{
    mPts.reset(position);
    return *this;
}

AudioCompositionParameters& AudioCompositionParameters::adjustPts(pts adjustment)
{
    if (mPts)
    {
        *mPts += adjustment;
    }
    return *this;
}

bool AudioCompositionParameters::hasPts() const
{
    return static_cast<bool>(mPts);
}

pts AudioCompositionParameters::getPts() const
{
    ASSERT(mPts);
    return *mPts;
}

AudioCompositionParameters& AudioCompositionParameters::determineChunkSize()
{
    ASSERT(!mChunkSize);
    pts position = getPts();
    samplecount chunkSize = (Convert::ptsToSamplesPerChannel(mSampleRate, position + 1) - Convert::ptsToSamplesPerChannel(mSampleRate, position)) * mNrChannels;
    mChunkSize.reset(chunkSize);
    return *this;
}

samplecount AudioCompositionParameters::getChunkSize() const
{
    ASSERT(mChunkSize);
    return *mChunkSize;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioCompositionParameters& obj)
{
    os  << &obj << '|' 
        << obj.mSampleRate << '|' 
        << obj.mNrChannels << '|' 
        << obj.mSpeed << '|'
        << obj.mPts << '|' 
        << obj.mChunkSize;
    return os;
}

} //namespace