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

#include "SequenceEvent.h"

#include "Track.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_VIDEO_TRACK,      EventAddVideoTracks,      TrackChange);
DEFINE_EVENT(EVENT_REMOVE_VIDEO_TRACK,   EventRemoveVideoTracks,   TrackChange);
DEFINE_EVENT(EVENT_ADD_AUDIO_TRACK,      EventAddAudioTracks,      TrackChange);
DEFINE_EVENT(EVENT_REMOVE_AUDIO_TRACK,   EventRemoveAudioTracks,   TrackChange);

TrackChange::TrackChange()
:   addedTracks()
,   addPosition()
,   removedTracks()
,   removePosition()
{
}

TrackChange::TrackChange(const Tracks& _addedTracks, const TrackPtr& _addPosition, const Tracks& _removedTracks, const TrackPtr& _removePosition)
:   addedTracks(_addedTracks)
,   addPosition(_addPosition)
,   removedTracks(_removedTracks)
,   removePosition(_removePosition)
{
}

TrackChange::TrackChange(const TrackChange& other)
:   addedTracks(other.addedTracks)
,   addPosition(other.addPosition)
,   removedTracks(other.removedTracks)
,   removePosition(other.removePosition)
{
}

std::ostream& operator<<(std::ostream& os, const TrackChange& obj)
{
    os << &obj << '|' << obj.addedTracks << '|' << obj.addPosition << '|' << obj.removedTracks << '|' << obj.removePosition;
    return os;
}

TrackChange TrackChange::make_inverted()
{
    return TrackChange(removedTracks,removePosition,addedTracks,addPosition);
}

} // namespace