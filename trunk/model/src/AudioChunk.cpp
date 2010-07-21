#include "AudioChunk.h"
#include "UtilLog.h"

const int AudioChunk::sBytesPerSample = 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioChunk::AudioChunk(boost::int16_t* buffer, int nChannels, unsigned int nSamples, double pts)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mTimeStamp(pts)
{
    /** @todo now we only used fixed stereo... */
    mBuffer = static_cast<boost::int16_t*>(malloc(mNrSamples * sBytesPerSample));
    memcpy(mBuffer, buffer, mNrSamples * sBytesPerSample);
}

AudioChunk::AudioChunk(int nChannels, unsigned int nSamples, double pts)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mTimeStamp(pts)
{
}

AudioChunk::~AudioChunk()
{
 //   free(mBuffer); /** /todo free the buffer?  */
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

double AudioChunk::getTimeStamp() const
{
    return mTimeStamp;
}

unsigned int AudioChunk::getNumberOfChannels() const
{
    return mNrChannels;
}

//////////////////////////////////////////////////////////////////////////
// DATA ACCESS
//////////////////////////////////////////////////////////////////////////

void AudioChunk::read(unsigned int samples)
{
    mNrReadSamples += samples;
}

boost::int16_t* AudioChunk::getUnreadSamples()
{
    ASSERT(mBuffer);
    return mBuffer + mNrReadSamples;

}

unsigned long AudioChunk::getUnreadSampleCount() const
{
    return mNrSamples - mNrReadSamples;
}

std::ostream& operator<< (std::ostream& os, const AudioChunk& obj)
{
    os  << &obj                         << "|" 
        << obj.getTimeStamp()           << "|" 
        << obj.getNumberOfChannels()    << "|" 
        << obj.getUnreadSampleCount();
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
