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

#pragma once

#include "Enums.h"

namespace test {

/// \return the given clip cast to a video clip
model::VideoClipPtr getVideoClip(model::IClipPtr clip);

/// \return the given clip cast to an audio clip
model::AudioClipPtr getAudioClip(model::IClipPtr clip);

/// \return the current scaling factor for the given (video) clip
/// \pre clip is a video clip
boost::rational<int> getScalingFactor(model::IClipPtr clip);

/// \return the current scaling type for the given (video) clip
/// \pre clip is a video clip
model::VideoScaling getScaling(model::IClipPtr clip);

/// \return the current alignment type for the given (video) clip
/// \pre clip is a video clip
model::VideoAlignment getAlignment(model::IClipPtr clip);

/// \return the current position type for the given (video) clip
/// \pre clip is a video clip
wxPoint getPosition(model::IClipPtr clip);

/// Unlink the clip from it's link (and vice versa)
/// \pre clip->getLink()
/// \post !clip->getLink() && !originalLink->getLink()
void Unlink(model::IClipPtr clip);

void AssertClipSpeed(model::IClipPtr clip, rational speed);
#define ASSERT_CLIP_SPEED(clip, speed) AssertClipSpeed(clip, speed)

} // namespace
