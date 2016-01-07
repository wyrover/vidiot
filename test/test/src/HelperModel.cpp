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

model::VideoKeyFramePtr DefaultVideoKeyFrame(model::IClipPtr clip)
{
    return boost::dynamic_pointer_cast<model::VideoKeyFrame>(getVideoClip(clip)->getDefaultKeyFrame());
}

std::pair<pts, model::VideoKeyFramePtr> VideoKeyFrame(model::IClipPtr clip, size_t index)
{
    std::map<pts, model::KeyFramePtr> keyFrames{ getVideoClip(clip)->getKeyFramesOfPerceivedClip() };
    ASSERT_NONZERO(keyFrames.size());
    std::map<pts, model::KeyFramePtr>::const_iterator it{ std::next(keyFrames.begin(), index) };
    ASSERT(it != keyFrames.end())(keyFrames)(index);
    return std::make_pair(it->first, boost::dynamic_pointer_cast<model::VideoKeyFrame>(it->second));
}

model::AudioKeyFramePtr DefaultAudioKeyFrame(model::IClipPtr clip)
{
    return boost::dynamic_pointer_cast<model::AudioKeyFrame>(getAudioClip(clip)->getDefaultKeyFrame());
}

std::pair<pts, model::AudioKeyFramePtr> AudioKeyFrame(model::IClipPtr clip, size_t index)
{
    std::map<pts, model::KeyFramePtr> keyFrames{ getAudioClip(clip)->getKeyFramesOfPerceivedClip() };
    ASSERT_NONZERO(keyFrames.size());
    std::map<pts, model::KeyFramePtr>::const_iterator it{ std::next(keyFrames.begin(), index) };
    ASSERT(it != keyFrames.end())(keyFrames)(index);
    return std::make_pair(it->first, boost::dynamic_pointer_cast<model::AudioKeyFrame>(it->second));
}

void Unlink(model::IClipPtr clip)
{
    model::SequencePtr sequence = getSequence();
    ASSERT_NONZERO(clip->getLink());
    ASSERT_NONZERO(clip->getLink()->getLink());
    util::thread::RunInMainAndWait([sequence,clip]()
    {
        (new gui::timeline::cmd::UnlinkClips(getSequence(), { clip, clip->getLink() }))->submit();
    });
}

void AssertClipSpeed(model::IClipPtr clip, rational64 speed)
{
    model::ClipIntervalPtr interval{ boost::dynamic_pointer_cast<model::ClipInterval>(clip) };
    ASSERT(interval != nullptr);
    ASSERT_EQUALS(interval->getSpeed(), speed);
}

} // namespace
