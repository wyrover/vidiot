#include "EmptyChunk.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyChunk::EmptyChunk(int nChannels, unsigned int nSamples, double pts)
:   AudioChunk(nChannels,nSamples,pts)
,   mInitialized(false)
{
}

EmptyChunk::~EmptyChunk()
{
    // Deallocation handled by base class
}

//////////////////////////////////////////////////////////////////////////
// DATA ACCESS
//////////////////////////////////////////////////////////////////////////

boost::int16_t* EmptyChunk::getUnreadSamples()
{
    if (!mInitialized)
    {
        mBuffer = static_cast<boost::int16_t*>(malloc(mNrSamples * AudioChunk::sBytesPerSample));
        memset(mBuffer, 0, mNrSamples * sBytesPerSample);
        mInitialized = true;
    }
    return AudioChunk::getUnreadSamples();
}
