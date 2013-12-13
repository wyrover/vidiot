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

#include "HelperModel.h"

#include "HelperThread.h"
#include "UtilLog.h"
#include "UnlinkClips.h"
#include "VideoClip.h"

namespace test {

model::VideoClipPtr getVideoClip(model::IClipPtr clip)
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(clip);
    ASSERT(videoclip);
    return videoclip;
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

void unlink(model::IClipPtr clip)
{
    model::SequencePtr sequence = getSequence();
    ASSERT_NONZERO(clip->getLink());
    ASSERT_NONZERO(clip->getLink()->getLink());
    RunInMainAndWait([sequence,clip]()
    {
        (new gui::timeline::command::UnlinkClips(getSequence(),boost::assign::list_of(clip)(clip->getLink())))->submit();
    });
}

} // namespace