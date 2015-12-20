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

#include "PositionInfo.h"

#include "Config.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MouseOnClipPosition);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

PointerPositionInfo::PointerPositionInfo()
    : onAudioVideoDivider(false)
    , track(model::TrackPtr())
    , trackPosition(0)
    , onTrackDivider(false)
    , clip(model::IClipPtr())
    , logicalclipposition(ClipInterior)
    , keyframe(boost::none)
{
}

//////////////////////////////////////////////////////////////////////////
// CLIP
//////////////////////////////////////////////////////////////////////////

model::IClipPtr PointerPositionInfo::getLogicalClip() const
{
    model::IClipPtr result{ clip };
    if (clip)
    {
        switch (logicalclipposition)
        {
        case ClipBegin:
            break;
        case ClipInterior:
            break;
        case ClipEnd:
            break;
        case TransitionBegin:
            break;
        case TransitionLeftClipInterior:
        case TransitionLeftClipEnd:
            result = clip->getPrev();
            break;
        case TransitionInterior:
            break;
        case TransitionRightClipBegin:
        case TransitionRightClipInterior:
            result = clip->getNext();
            break;
        case TransitionEnd:
            break;
        default:
            FATAL("Unexpected logical clip position.");
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const PointerPositionInfo& obj)
{
    os  << &obj << '|' 
        << obj.onAudioVideoDivider << '|' 
        << obj.track << '|' 
        << obj.trackPosition << '|' 
        << obj.onTrackDivider << '|' 
        << obj.clip << '|' 
        << obj.logicalclipposition << '|'
        << obj.keyframe;
    return os;
}

}} // namespace