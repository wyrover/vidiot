#include "EmptyChunk.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyChunk::EmptyChunk(int nChannels, unsigned int nSamples, pts position)
:   AudioChunk(nChannels,nSamples,position)
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

sample* EmptyChunk::getUnreadSamples()
{
    if (!mInitialized)
    {
        mBuffer = static_cast<sample*>(malloc(mNrSamples * AudioChunk::sBytesPerSample));
        memset(mBuffer, 0, mNrSamples * sBytesPerSample);
        mInitialized = true;
    }
    return AudioChunk::getUnreadSamples();
}

} // namespace
