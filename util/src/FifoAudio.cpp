#include "FifoAudio.h"

const int AudioChunk::sBytesPerSample = 2;

AudioChunk::AudioChunk(boost::int16_t* buffer, int nChannels, unsigned int nSamples, double pts)
:   mBuffer(0)
,   mNrChannels(nChannels)
,   mNrSamples(nSamples)
,   mNrReadSamples(0)
,   mTimeStamp(pts)
{
    mBuffer = static_cast<boost::int16_t*>(malloc(mNrSamples * sBytesPerSample));
    memcpy(mBuffer, buffer, mNrSamples * sBytesPerSample);
}

AudioChunk::~AudioChunk()
{
 //   free(mBuffer); /** /todo free the buffer?  */
}

double AudioChunk::getTimeStamp() const
{
    return mTimeStamp;
}

unsigned int AudioChunk::getNumberOfChannels() const
{
    return mNrChannels;
}

void AudioChunk::read(unsigned int samples)
{
    mNrReadSamples += samples;
}

boost::int16_t* AudioChunk::getUnreadSamples() const
{
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
