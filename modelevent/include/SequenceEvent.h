#ifndef MODEL_SEQUENCE_EVENT_H
#define MODEL_SEQUENCE_EVENT_H

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
    TrackChange(Tracks _addedTracks, TrackPtr _addPosition = TrackPtr(), Tracks _removedTracks = Tracks(), TrackPtr _removePosition = TrackPtr());

    /// Copy constructor
    TrackChange(const TrackChange& other);

    friend std::ostream& operator<<( std::ostream& os, const TrackChange& obj );

    /// \return new object that is the inverse of this object.
    /// This means that all additions and removals are interchanged.
    TrackChange make_inverted();
};

DECLARE_EVENT(EVENT_ADD_VIDEO_TRACK,      EventAddVideoTracks,      TrackChange);
DECLARE_EVENT(EVENT_REMOVE_VIDEO_TRACK,   EventRemoveVideoTracks,   TrackChange);
DECLARE_EVENT(EVENT_ADD_AUDIO_TRACK,      EventAddAudioTracks,      TrackChange);
DECLARE_EVENT(EVENT_REMOVE_AUDIO_TRACK,   EventRemoveAudioTracks,   TrackChange);

} // namespace

#endif // MODEL_SEQUENCE_EVENT_H