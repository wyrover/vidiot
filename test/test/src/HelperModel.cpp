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

#include "Test.h"

namespace test {

model::VideoClipPtr getVideoClip(model::IClipPtr clip)
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(clip);
    ASSERT(videoclip);
    return videoclip;
}

model::AudioClipPtr getAudioClip(model::IClipPtr clip)
{
    model::AudioClipPtr audioclip = boost::dynamic_pointer_cast<model::AudioClip>(clip);
    ASSERT(audioclip);
    return audioclip;
}

boost::rational<int> getScalingFactor(model::IClipPtr clip)
{
    return getVideoClip(clip)->getScalingFactor();
};

model::VideoScaling getScaling(model::IClipPtr clip)
{
    return getVideoClip(clip)->getScaling();
};

model::VideoAlignment getAlignment(model::IClipPtr clip)
{
    return getVideoClip(clip)->getAlignment();
}

wxPoint getPosition(model::IClipPtr clip)
{
    return getVideoClip(clip)->getPosition();
}

void Unlink(model::IClipPtr clip)
{
    model::SequencePtr sequence = getSequence();
    ASSERT_NONZERO(clip->getLink());
    ASSERT_NONZERO(clip->getLink()->getLink());
    util::thread::RunInMainAndWait([sequence,clip]()
    {
        (new gui::timeline::command::UnlinkClips(getSequence(), { clip, clip->getLink() }))->submit();
    });
}

void AssertClipSpeed(model::IClipPtr clip, rational speed)
{
    model::ClipIntervalPtr interval{ boost::dynamic_pointer_cast<model::ClipInterval>(clip) };
    ASSERT(interval != nullptr);
    ASSERT_EQUALS(interval->getSpeed(), speed);
}

} // namespace