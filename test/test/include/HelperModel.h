// Copyright 2013-2016 Eric Raijmakers.
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

#include "Enums.h"

namespace test {

/// \return the given clip cast to a video clip
model::VideoClipPtr getVideoClip(model::IClipPtr clip);

/// \return the given clip cast to an audio clip
model::AudioClipPtr getAudioClip(model::IClipPtr clip);

/// \return the default key frame for a clip
/// \pre clip is a video clip
model::VideoKeyFramePtr DefaultVideoKeyFrame(model::IClipPtr clip);

/// \return the key frame at the given position
/// \pre clip is a video clip
std::pair<pts, model::VideoKeyFramePtr> VideoKeyFrame(model::IClipPtr clip, size_t index);

/// \return the default key frame for a clip
/// \pre clip is an audio clip
model::AudioKeyFramePtr DefaultAudioKeyFrame(model::IClipPtr clip);

/// \return the key frame at the given position
/// \pre clip is an audio clip
std::pair<pts, model::AudioKeyFramePtr> AudioKeyFrame(model::IClipPtr clip, size_t index);

/// Unlink the clip from it's link (and vice versa)
/// \pre clip->getLink()
/// \post !clip->getLink() && !originalLink->getLink()
void Unlink(model::IClipPtr clip);

void AssertClipSpeed(model::IClipPtr clip, rational64 speed);
#define ASSERT_CLIP_SPEED(clip, speed) AssertClipSpeed(clip, speed)

} // namespace
