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

#include "AudioChunk.h"

#include "UtilLog.h"

namespace model {

const int AudioChunk::sBytesPerSample = 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioChunk::AudioChunk(sample* buffer, int nChannels, samplecount nSamples)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mNrSkippedSamples(0)
{
    mBuffer = new sample[mNrSamples];
    if (buffer)
    {
        memcpy(mBuffer, buffer, mNrSamples * sBytesPerSample);
    }
}

AudioChunk::AudioChunk(int nChannels, samplecount nSamples, bool allocate)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mNrSkippedSamples(0)
{
    // todo calloc here and delete[] in destructor???
    if (allocate)
    {
        mBuffer = static_cast<sample*>(calloc( mNrSamples, sBytesPerSample ));
    }
}

AudioChunk::~AudioChunk()
{
    if (mBuffer)
    {
        delete[] mBuffer;
        mBuffer = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

unsigned int AudioChunk::getNumberOfChannels() const
{
    return mNrChannels;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

samplecount AudioChunk::extract(uint16_t* dst, samplecount requested)
{
    samplecount actual = min(getUnreadSampleCount(),requested);
    memcpy(dst,getUnreadSamples(), actual * sBytesPerSample);
    read(actual);
    return actual;
}

void AudioChunk::read(samplecount samples)
{
    mNrReadSamples += samples;
    ASSERT_LESS_THAN_EQUALS(mNrReadSamples, mNrSamples);
}

sample* AudioChunk::getBuffer()
{
    ASSERT(mBuffer);
    return mBuffer;
}

sample* AudioChunk::getUnreadSamples()
{
    ASSERT(mBuffer);
    return mBuffer + mNrReadSamples;
}

samplecount AudioChunk::getUnreadSampleCount() const
{
    samplecount result = mNrSamples - mNrSkippedSamples - mNrReadSamples;
    ASSERT_MORE_THAN_EQUALS_ZERO(result);
    return result;
}

void AudioChunk::setAdjustedLength(samplecount adjustedLength)
{
    ASSERT_LESS_THAN_EQUALS(adjustedLength,mNrSamples);
    mNrSkippedSamples = mNrSamples - adjustedLength;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const AudioChunk& obj)
{
    os  << &obj                     << "|"
        << obj.mNrChannels          << "|"
        << obj.mNrSamples           << "|"
        << obj.mNrSkippedSamples    << "|"
        << obj.mNrReadSamples;
    return os;
}

std::ostream& operator<< (std::ostream& os, const AudioChunkPtr obj)
{
    if (obj)
    {
        os << *obj;
    }
    else
    {
        os << "0";
    }
    return os;
}

} // namespace