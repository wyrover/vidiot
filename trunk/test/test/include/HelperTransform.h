// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef HELPER_TRANSFORM_H
#define HELPER_TRANSFORM_H

#include <boost/shared_ptr.hpp>
#include "UtilEnumSelector.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoTrack;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
class VideoTransition;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;
class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
}

namespace test {

template <class ITEMTYPE>
void ClickOnEnumSelector(EnumSelector<ITEMTYPE>* widget, ITEMTYPE value)
{
    ClickTopLeft(widget);
    Type(WXK_HOME);
    for (int step = 0; step < widget->getIndex(value); ++step)
    {
        Type(WXK_DOWN);
    }
    Type(WXK_RETURN);
    ASSERT_EQUALS(widget->getValue(),value);
    waitForIdle();
}

void ResizeClip(model::IClipPtr clip, wxSize boundingbox);

} // namespace

#endif