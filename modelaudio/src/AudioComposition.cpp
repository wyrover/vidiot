// Copyright 2013-2015 Eric Raijmakers.
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
#include "UtilLogStl.h"
#include "UtilVector.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioComposition::AudioComposition(const AudioCompositionParameters& parameters)
    : mChunks()
    , mParameters(parameters)
{
}

AudioComposition::AudioComposition(const AudioComposition& other)
    : mChunks(other.mChunks)
    , mParameters(other.mParameters)
{
}

AudioComposition::~AudioComposition()
{
}

//////////////////////////////////////////////////////////////////////////
// COMPOSITION
//////////////////////////////////////////////////////////////////////////

void AudioComposition::add(const AudioChunkPtr& chunk)
{
    if (chunk &&
        !chunk->isA<model::EmptyChunk>())
    {
        // Skip empty chunks.
        mChunks.push_back(chunk);
        ASSERT_EQUALS(chunk->getUnreadSampleCount(), mParameters.getChunkSize());
    }
}

AudioChunkPtr AudioComposition::generate()
{
    AudioChunkPtr result;

    if (mChunks.empty())
    {
        result = boost::make_shared<EmptyChunk>(mParameters.getNrChannels(), mParameters.getChunkSize());
    }

    if (mChunks.size() == 1)
    {
        AudioChunkPtr front = mChunks.front();
        if (true)
        {
            // Performance optimization: if only one chunk is rendered, return that chunk, but only if the chunk requires no 'processing'.
            result = front;
        }
    }

    if (!result)
    {
        samplecount chunkSize = mParameters.getChunkSize();
        result = boost::make_shared<AudioChunk>(mParameters.getNrChannels(), chunkSize, true, true); // Fills with 0

        for (AudioChunkPtr inputChunk : mChunks)
        {
            ASSERT(inputChunk);
            ASSERT(!inputChunk->isA<EmptyChunk>());
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
            inputChunk->read(chunkSize);
        }
    }

    ASSERT_NONZERO(result);
    if (mParameters.hasPts())
    {
        result->setPts(mParameters.getPts());
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

std::ostream& operator<<(std::ostream& os, const AudioComposition& obj)
{
    os << &obj << '|' << obj.mChunks;
    return os;
}

} //namespace
