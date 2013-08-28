// Copyright 2013 Eric Raijmakers.
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
    ,   mInputChunkReturnedAsOutput(false)
{
    VAR_DEBUG(this);
}

AudioComposition::AudioComposition(const AudioComposition& other)
    :   mChunks(other.mChunks)
    ,   mParameters(other.mParameters)
    ,   mInputChunkReturnedAsOutput(false)
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
        return boost::make_shared<EmptyChunk>(mParameters.getNrChannels(), mParameters.ptsToSamples(1));
    }

    if (mChunks.size() == 1)
    {
        AudioChunkPtr front = mChunks.front();
        if (true)
        {
            // Performance optimization: if only one chunk is rendered, return that chunk, but only if the chunk requires no 'processing'.
            mInputChunkReturnedAsOutput = true;
            return front;
        }
    }

    samplecount chunkSize = std::numeric_limits<samplecount>::max();
    BOOST_FOREACH( AudioChunkPtr inputChunk, mChunks )
    {
        if (chunkSize > inputChunk->getUnreadSampleCount() )
        {
            chunkSize = inputChunk->getUnreadSampleCount();
        }
    }
    ASSERT_DIFFERS(chunkSize, std::numeric_limits<samplecount>::max());

    VAR_DEBUG(chunkSize);
    AudioChunkPtr result = boost::make_shared<AudioChunk>(mParameters.getNrChannels(), chunkSize, true); // Fills with 0

    BOOST_FOREACH(AudioChunkPtr inputChunk, mChunks)
    {
        if (inputChunk)
        {
            if (!inputChunk->isA<EmptyChunk>())
            {
                sample max = std::numeric_limits<sample>::max();
                sample* inputSample = inputChunk->getUnreadSamples(); // NOT: getBuffer()
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
            inputChunk->read(chunkSize);
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

bool AudioComposition::wasInputChunkReturnedAsOutput() const
{
    return mInputChunkReturnedAsOutput;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioComposition& obj )
{
    os << &obj << '|' << obj.mChunks << '|' << obj.mInputChunkReturnedAsOutput;
    return os;
}

} //namespace