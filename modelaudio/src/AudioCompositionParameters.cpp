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

#include "AudioCompositionParameters.h"

#include "Config.h"
#include "Convert.h"
#include "UtilLogBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters::AudioCompositionParameters()
    : mSampleRate(Config::ReadLong(Config::sPathDefaultAudioSampleRate))
    , mNrChannels(Config::ReadLong(Config::sPathDefaultAudioChannels))
    , mPts(boost::none)
    , mChunkSize(boost::none)
{
}

AudioCompositionParameters::AudioCompositionParameters(const AudioCompositionParameters& other)
    : mSampleRate(other.mSampleRate)
    , mNrChannels(other.mNrChannels)
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
    samplecount chunkSize = Convert::ptsToSamples(mSampleRate, mNrChannels, position + 1) - Convert::ptsToSamples(mSampleRate, mNrChannels, position);
    ASSERT_ZERO(chunkSize % mNrChannels); // Ensure that the data for all speakers is there... If this assert ever fails: maybe there's file formats in which the data for a frame is 'truncated'?
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
    os << &obj << '|' << obj.mSampleRate << '|' << obj.mNrChannels << '|' << obj.mPts << '|' << obj.mChunkSize;
    return os;
}

} //namespace