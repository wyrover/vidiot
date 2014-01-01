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

#include "UtilEvent.h"

namespace model {

struct MoveParameter
{
    /// Into this track the moved clips need to be inserted.
    TrackPtr addTrack;

    /// The moved clips must be inserted before this clip.
    /// If this is an uninitialized pointer, then the clips need
    /// to be inserted at the end of the track.
    IClipPtr addPosition;

    /// Consecutive list of clips to be added to this track
    /// (they'll be joined together exactly in the same order).
    /// These clips may also be a part of removeClips.
    /// If multiple consecutive (but not directly connected) lists
    /// need to be added, add multiple MoveParameter objects.
    IClips addClips;

    /// From this track the moved clips need to be removed.
    TrackPtr removeTrack;

    /// In case of undo, the removed clips must be reinserted
    /// before this clip. If this is an uninitialized pointer,
    /// then the insertion is at the end of the track.
    IClipPtr removePosition;

    /// Any clips to be removed from this track
    /// These clips may also be a part of addClips.
    /// This needs to be a consecutive list of clips,
    /// thus without 'emptyness' in between. If multiple,
    /// not directly connected clip lists need to be removed,
    /// instantiate two MoveParameter objects (one for each
    /// list).
    IClips removeClips;

    /// Empty constructor (used to avoid 'no appropriate default ctor' error messages after I added the other constructor).
    MoveParameter();

    /// Helper constructor to initialize all members in one statement.
    MoveParameter(TrackPtr _addTrack, IClipPtr _addPosition, IClips _addClips, TrackPtr _removeTrack = TrackPtr(), IClipPtr _removePosition = IClipPtr(), IClips _removeClips = IClips());

    /// Copy constructor
    MoveParameter(const MoveParameter& other);

    /// \return new move object that is the inverse of this object.
    /// This means that all additions and removals are interchanged.
    MoveParameterPtr make_inverted();
};

std::ostream& operator<<( std::ostream& os, const MoveParameter& obj );

DECLARE_EVENT(EVENT_ADD_CLIPS,          EventAddClips,              MoveParameter);
DECLARE_EVENT(EVENT_REMOVE_CLIPS,       EventRemoveClips,           MoveParameter);
DECLARE_EVENT(EVENT_HEIGHT_CHANGED,     EventHeightChanged,         int);

} // namespace