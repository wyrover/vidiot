#ifndef EMPTY_CHUNK_H
#define EMPTY_CHUNK_H

#include "AudioChunk.h"

namespace model {

// FORWARD DECLARATIONS
class EmptyChunk;
typedef boost::shared_ptr<EmptyChunk> EmptyChunkPtr;

class EmptyChunk : public AudioChunk
{
public:
    
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyChunk(int nChannels, unsigned int nSamples, double pts);
    
    virtual ~EmptyChunk();

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /**
     * Override from AudioChunk. When this method is called for the first time,
     * then the zeroed buffer is generated.
     */
    boost::int16_t* getUnreadSamples();

private:

    bool mInitialized;
};

} // namespace

#endif // EMPTY_CHUNK_H