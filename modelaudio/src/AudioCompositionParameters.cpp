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

#include "AudioCompositionParameters.h"

#include "Config.h"
#include "Convert.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters::AudioCompositionParameters()
    :   mSampleRate(Config::ReadLong(Config::sPathDefaultAudioSampleRate))
    ,   mNrChannels(Config::ReadLong(Config::sPathDefaultAudioChannels))
{
}

AudioCompositionParameters::AudioCompositionParameters(const AudioCompositionParameters& other)
    :   mSampleRate(other.mSampleRate)
    ,   mNrChannels(other.mNrChannels)
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

samplecount AudioCompositionParameters::getChunkSize() const
{
    return ptsToSamples(1);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION HELPERS
//////////////////////////////////////////////////////////////////////////

int AudioCompositionParameters::ptsToSamples(pts position) const
{
    return Convert::ptsToSamples(position, mSampleRate, mNrChannels);
}

pts AudioCompositionParameters::samplesToPts(int nFrames) const
{
    return Convert::samplesToPts(nFrames, mSampleRate, mNrChannels);
}

int AudioCompositionParameters::samplesToFrames(int nSamples) const
{
    return Convert::samplesToFrames(mNrChannels, nSamples);
}

int AudioCompositionParameters::framesToSamples(int nFrames) const
{
    return Convert::framesToSamples(mNrChannels, nFrames);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioCompositionParameters& obj )
{
    os << &obj << '|' << obj.mSampleRate << '|' << obj.mNrChannels;
    return os;
}

} //namespace