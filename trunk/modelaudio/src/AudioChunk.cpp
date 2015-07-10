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

#include "AudioChunk.h"

#include "UtilLog.h"
#include "UtilLogBoost.h"

namespace model {

const int AudioChunk::sBytesPerSample = 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioChunk::AudioChunk(int nChannels, samplecount nSamples, bool allocate, bool zero, sample* buffer)
    : mBuffer(0)
    , mNrChannels(nChannels)
    , mNrSamples(nSamples)
    , mNrReadSamples(0)
    , mNrSkippedSamples(0)
    , mPts(boost::none)
{
    ASSERT_IMPLIES(zero,        allocate && buffer == 0);
    ASSERT_IMPLIES(buffer != 0, allocate && !zero);
    ASSERT_ZERO(nSamples % nChannels)(nSamples)(nChannels);

    if (allocate)
    {
        if (zero)
        {
            mBuffer = static_cast<sample*>(calloc( mNrSamples, sBytesPerSample ));
        }
        else
        {
            mBuffer = static_cast<sample*>(malloc( mNrSamples * sBytesPerSample ));
        }
        ASSERT_NONZERO(mBuffer);
        if (buffer)
        {
            memcpy(mBuffer, buffer, mNrSamples * sBytesPerSample);
        }
    }
}

AudioChunk::~AudioChunk()
{
    if (mBuffer)
    {
        free(mBuffer);
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

samplecount AudioChunk::extract(sample* dst, samplecount requested)
{
    samplecount actual = min(getUnreadSampleCount(),requested);
    memcpy(dst,getUnreadSamples(), actual * sBytesPerSample);
    read(actual);
    return actual;
}

void AudioChunk::read(samplecount samples)
{
    ASSERT_LESS_THAN_EQUALS(mNrReadSamples + mNrSkippedSamples + samples, mNrSamples)(mNrSamples)(mNrReadSamples)(mNrSkippedSamples)(samples);
    mNrReadSamples += samples;
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

samplecount AudioChunk::getReadSampleCount() const
{
    samplecount result = mNrReadSamples;
    ASSERT_MORE_THAN_EQUALS_ZERO(result);
    return result;
}

samplecount AudioChunk::getUnreadSampleCount() const
{
    ASSERT_LESS_THAN_EQUALS(mNrReadSamples + mNrSkippedSamples, mNrSamples)(mNrSamples)(mNrReadSamples)(mNrSkippedSamples);
    samplecount result = mNrSamples - mNrSkippedSamples - mNrReadSamples;
    return result;
}

void AudioChunk::setAdjustedLength(samplecount adjustedLength)
{
    ASSERT_ZERO(mNrSkippedSamples); // Can only set once
    ASSERT_ZERO(mNrReadSamples); // Can only set when nothing is read yet
    ASSERT_LESS_THAN_EQUALS(adjustedLength,mNrSamples);
    mNrSkippedSamples = mNrSamples - adjustedLength;
    samplecount total = mNrSamples - mNrSkippedSamples - mNrReadSamples;
    ASSERT_ZERO(total % mNrChannels)(mNrSamples)(mNrSkippedSamples)(mNrReadSamples)(mNrChannels);
    ASSERT_LESS_THAN_EQUALS(mNrReadSamples + mNrSkippedSamples, mNrSamples)(mNrSamples)(mNrReadSamples)(mNrSkippedSamples)(adjustedLength);
}

pts AudioChunk::getPts() const
{
    ASSERT(mPts);
    return *mPts;
}

void AudioChunk::setPts(pts position)
{
    mPts.reset(position);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioChunk& obj)
{
    os  << &obj                     << "|"
        << obj.mPts                 << "|"
        << obj.mNrChannels          << "|"
        << obj.mNrSamples           << "|"
        << obj.mNrSkippedSamples    << "|"
        << obj.mNrReadSamples       << "|"
        << typeid(obj).name();
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
