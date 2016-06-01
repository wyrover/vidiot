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

#pragma once

namespace test {

/// Get a chunk from the given clip. A clone of the given clip is first
/// created of the clip, to ensure that the clip itself is not modified.
/// \param clip video clip for which a frame is retrieved
/// \param offset number of the frame
/// \note First frame of clip equals offset '0', last frame equals offset length - 1
/// \pre clip->isA<model::VideoClip>()
/// \pre 0 <= offset < clip->getLength()
model::AudioChunkPtr GetChunk(model::IClipPtr clip, pts offset);

/// \return first chunk from given clip.
/// \param clip audio clip for which a frame is retrieved
model::AudioChunkPtr FirstChunk(model::IClipPtr clip);

/// \return last chunk from given clip.
/// \param clip audio clip for which a frame is retrieved
model::AudioChunkPtr LastChunk(model::IClipPtr clip);

void assertChunksEqual(model::AudioChunkPtr chunk1, model::AudioChunkPtr chunk2);
#define ASSERT_CHUNKS_EQUAL(chunk1, chunk2) assertChunksEqual(chunk1, chunk2)

} // namespace
