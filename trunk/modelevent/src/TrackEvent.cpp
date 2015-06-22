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

#include "TrackEvent.h"

#include "UtilLogStl.h"
#include "IClip.h"

namespace model {

DEFINE_EVENT(EVENT_CLIPS_REPLACED,      EventClipsReplaced,     MoveParameter);

MoveParameter::MoveParameter()
:   addTrack()
,   addPosition()
,   addClips()
,   removeTrack()
,   removePosition()
,   removeClips()
{
}

MoveParameter::MoveParameter(const TrackPtr& _addTrack, const IClipPtr& _addPosition, const IClips& _addClips, const TrackPtr& _removeTrack, const IClipPtr& _removePosition, const IClips& _removeClips)
:   addTrack(_addTrack)
,   addPosition(_addPosition)
,   addClips(_addClips)
,   removeTrack(_removeTrack)
,   removePosition(_removePosition)
,   removeClips(_removeClips)
{
}

MoveParameter::MoveParameter(const MoveParameter& other)
:   addTrack(other.addTrack)
,   addPosition(other.addPosition)
,   addClips(other.addClips)
,   removeTrack(other.removeTrack)
,   removePosition(other.removePosition)
,   removeClips(other.removeClips)
{
}

MoveParameterPtr MoveParameter::make_inverted()
{
    return boost::make_shared<MoveParameter>(removeTrack,removePosition,removeClips,addTrack,addPosition,addClips);
}

std::ostream& operator<<(std::ostream& os, const MoveParameter& obj)
{
    os << obj.removeTrack << '|' << obj.removePosition << '|' << obj.removeClips << '|' << obj.addTrack << '|' << obj.addPosition << '|' << obj.addClips;
    return os;
}

} // namespace