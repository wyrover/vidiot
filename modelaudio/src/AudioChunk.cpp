#include "AudioChunk.h"

#include "UtilLog.h"

namespace model {

const int AudioChunk::sBytesPerSample = 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioChunk::AudioChunk(sample* buffer, int nChannels, samplecount nSamples, pts position)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mNrSkippedSamples(0)
,   mPts(position)
{
    mBuffer = new sample[mNrSamples];
    if (buffer)
    {
        memcpy(mBuffer, buffer, mNrSamples * sBytesPerSample);
    }
}

AudioChunk::AudioChunk(int nChannels, samplecount nSamples, pts position)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mNrSkippedSamples(0)
,   mPts(position)
{
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

pts AudioChunk::getPts() const
{
    return mPts;
}

void AudioChunk::setPts(pts position)
{
    VAR_DEBUG(position);
    mPts = position;
}

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
    return mNrSamples - mNrSkippedSamples - mNrReadSamples;
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
        << obj.mPts                 << "|"
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