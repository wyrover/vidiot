#include "AudioComposition.h"

#include "Constants.h"
#include "Convert.h"
#include "EmptyChunk.h"
#include "Properties.h"
#include "UtilList.h"
#include "UtilLogStl.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioComposition::AudioComposition(const AudioCompositionParameters& parameters)
    :   mChunks()
    ,   mParameters(parameters)
{
    VAR_DEBUG(this);
}

AudioComposition::AudioComposition(const AudioComposition& other)
    :   mChunks(other.mChunks)
    ,   mParameters(other.mParameters)
{
}

AudioComposition::~AudioComposition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// COMPOSITION
//////////////////////////////////////////////////////////////////////////

void AudioComposition::add(AudioChunkPtr chunk)
{
    if (chunk && !chunk->isA<EmptyChunk>())
    {
        // Skip empty chunks.
        mChunks.push_back(chunk);
    }
}

void AudioComposition::replace(AudioChunkPtr oldChunk, AudioChunkPtr newChunk)
{
    UtilList<model::AudioChunkPtr>(mChunks).replace(oldChunk,newChunk);
}

AudioChunkPtr AudioComposition::generate()
{
    if (mChunks.empty())
    {
        return boost::make_shared<EmptyChunk>(mParameters.getNrChannels(), mParameters.ptsToSamples(1), 0);
    }

    if (mChunks.size() == 1)
    {
        AudioChunkPtr front = mChunks.front();
        if (true)
        {
            // Performance optimization: if only one chunk is rendered, return that chunk, but only if the chunk requires no 'processing'.
            return front;
        }
    }

    return mChunks.front();
    //BOOST_FOREACH( AudioChunkPtr chunk, mChunks )
    //{
    //}

    //AudioChunkPtr result = boost::make_shared<AudioChunk>(compositeImage,0);
    //return result;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters AudioComposition::getParameters() const
{
    return mParameters;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioComposition& obj )
{
    os << &obj << '|' << obj.mChunks;
    return os;
}

} //namespace