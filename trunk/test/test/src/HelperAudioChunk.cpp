// Copyright 2015-2016 Eric Raijmakers.
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

#include "Test.h"

namespace test {

model::AudioChunkPtr GetChunk(model::IClipPtr clip, pts offset)
{
    ASSERT_MORE_THAN_EQUALS_ZERO(offset);
    ASSERT_LESS_THAN(offset, clip->getLength());
    model::IClipPtr clone{ make_cloned<model::IClip>(clip) };
    model::AudioClipPtr audioclip{ getAudioClip(clone) };
    ASSERT_NONZERO(audioclip);
    audioclip->moveTo(offset);
    return util::thread::RunInMainReturning<model::AudioChunkPtr>([audioclip]
    {
        model::AudioCompositionParameters parameters; parameters.setPts(0).determineChunkSize();
        model::AudioChunkPtr result = audioclip->getNextAudio(parameters);
        ASSERT_NONZERO(result);
        return result;
    });
}

model::AudioChunkPtr FirstChunk(model::IClipPtr clip)
{
    return GetChunk(clip, 0);
}

model::AudioChunkPtr LastChunk(model::IClipPtr clip)
{
    return GetChunk(clip, clip->getLength() - 1);
}

void assertChunksEqual(model::AudioChunkPtr chunk1, model::AudioChunkPtr chunk2)
{
    sample* cur = chunk1->getUnreadSamples();
    sample* ref = chunk2->getUnreadSamples();
    sample* last = cur + 100;
    //double v{ volume / 100.0 };
    while (cur < last)
    {
        sample c = *cur;
        sample r{ *ref };
        //model::adjustSampleVolume(v, r);
        ASSERT_EQUALS(c,r);
        cur++;
        ref++;
    }
}

} // namespace
