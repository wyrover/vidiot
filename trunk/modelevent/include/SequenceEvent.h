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

#include "UtilEvent.h"

namespace model {

struct TrackChange
{
    Tracks addedTracks;

    /// The moved tracks must be inserted before this track.
    /// If this is an uninitialized pointer, then the tracks need
    /// to be inserted at the end.
    TrackPtr addPosition;

    Tracks removedTracks;

    /// In case of undo, the removed tracks must be reinserted
    /// before this track.If this is an uninitialized pointer,
    /// then the tracks need to be inserted at the end.
    TrackPtr removePosition;

    /// Empty constructor (used to avoid 'no appropriate default ctor' error messages after I added the other constructor).
    TrackChange();

    /// Helper constructor to initialize all members in one statement.
    /// Per default, when only supplying a list of tracks to be added, these
    /// are added to the end.
    TrackChange(
        const Tracks& _addedTracks,
        const TrackPtr& _addPosition = TrackPtr(),
        const Tracks& _removedTracks = Tracks(),
        const TrackPtr& _removePosition = TrackPtr());

    /// Copy constructor
    TrackChange(const TrackChange& other);

    friend std::ostream& operator<<(std::ostream& os, const TrackChange& obj);

    /// \return new object that is the inverse of this object.
    /// This means that all additions and removals are interchanged.
    TrackChange make_inverted();
};

DECLARE_EVENT(EVENT_ADD_VIDEO_TRACK,      EventAddVideoTracks,      TrackChange);
DECLARE_EVENT(EVENT_REMOVE_VIDEO_TRACK,   EventRemoveVideoTracks,   TrackChange);
DECLARE_EVENT(EVENT_ADD_AUDIO_TRACK,      EventAddAudioTracks,      TrackChange);
DECLARE_EVENT(EVENT_REMOVE_AUDIO_TRACK,   EventRemoveAudioTracks,   TrackChange);

} // namespace
