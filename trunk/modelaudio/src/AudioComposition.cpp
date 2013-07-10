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
    int chunkSize = mParameters.getChunkSize();

    if (mChunks.empty())
    {
        return boost::make_shared<EmptyChunk>(mParameters.getNrChannels(), chunkSize, 0);
    }

    if (mChunks.size() == 1)
    {
        AudioChunkPtr front = mChunks.front();
        if (true)
        {
            ASSERT_EQUALS(front->getUnreadSampleCount(), chunkSize);
            // Performance optimization: if only one chunk is rendered, return that chunk, but only if the chunk requires no 'processing'.
            return front;
        }
    }
    VAR_DEBUG(chunkSize);
    AudioChunkPtr result = boost::make_shared<AudioChunk>(mParameters.getNrChannels(), chunkSize); // Fills with 0

    BOOST_FOREACH(AudioChunkPtr inputChunk, mChunks)
    {
        ASSERT_EQUALS(inputChunk->getUnreadSampleCount(), mParameters.ptsToSamples(1));
        sample max = std::numeric_limits<sample>::max();
        sample* inputSample = inputChunk->getBuffer();
        sample* resultingSample = result->getBuffer();
        for (int nSample = 0; nSample < chunkSize; ++nSample)
        {
            if (*inputSample > max - *resultingSample)
            {
                // Overflow
                *resultingSample = max;
            }
            else
            {
                *resultingSample += *inputSample;
            }

            inputSample++;
            resultingSample++;
        }
    }

    return result;
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