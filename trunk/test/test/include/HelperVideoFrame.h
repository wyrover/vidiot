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

/// Get a frame from the given clip. A clone of the given clip is first
/// created of the clip, to ensure that the clip itself is not modified.
/// \param clip video clip for which a frame is retrieved
/// \param offset number of the frame
/// \note First frame of clip equals offset '0', last frame equals offset length - 1
/// \pre clip->isA<model::VideoClip>()
/// \pre 0 <= offset < clip->getLength()
model::VideoFramePtr GetFrame(model::IClipPtr clip, pts offset);

/// \return first frame from given clip.
/// \param clip video clip for which a frame is retrieved
model::VideoFramePtr FirstFrame(model::IClipPtr clip);

/// \return last frame from given clip.
/// \param clip video clip for which a frame is retrieved
model::VideoFramePtr LastFrame(model::IClipPtr clip);

void assertFramesEqual(model::VideoFramePtr frame1, model::VideoFramePtr frame2);
#define ASSERT_FRAMES_EQUAL(frame1, frame2) assertFramesEqual(frame1, frame2)

} // namespace
