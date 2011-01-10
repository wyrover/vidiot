#include "AudioChunk.h"
#include "UtilLog.h"

namespace model {

const int AudioChunk::sBytesPerSample = 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioChunk::AudioChunk(boost::int16_t* buffer, int nChannels, samples_t nSamples, pts position)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mNrSkippedSamples(0)
,   mPts(position)
{
    /** @todo now we only used fixed stereo... */
    mBuffer = static_cast<boost::int16_t*>(malloc(mNrSamples * sBytesPerSample));
    if (buffer)
    {
        memcpy(mBuffer, buffer, mNrSamples * sBytesPerSample);
    }
}

AudioChunk::AudioChunk(int nChannels, samples_t nSamples, pts position)
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
 //   free(mBuffer); /** /todo free the buffer?  */
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

pts AudioChunk::getPts() const
{
    return mPts;
}

unsigned int AudioChunk::getNumberOfChannels() const
{
    return mNrChannels;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void AudioChunk::read(samples_t samples)
{
    mNrReadSamples += samples;
}

boost::int16_t* AudioChunk::getBuffer()
{
    ASSERT(mBuffer);
    return mBuffer;
}

boost::int16_t* AudioChunk::getUnreadSamples()
{
    ASSERT(mBuffer);
    return mBuffer + mNrReadSamples;

}

samples_t AudioChunk::getUnreadSampleCount() const
{
    return mNrSamples - mNrSkippedSamples - mNrReadSamples;
}

void AudioChunk::setAdjustedLength(samples_t adjustedLength)
{
    ASSERT(adjustedLength < mNrSamples)(adjustedLength)(mNrSamples);
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
