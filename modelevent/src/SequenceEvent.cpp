#include "SequenceEvent.h"

#include "UtilLogStl.h"
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

TrackChange::TrackChange(Tracks _addedTracks, TrackPtr _addPosition, Tracks _removedTracks, TrackPtr _removePosition)
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

std::ostream& operator<<( std::ostream& os, const TrackChange& obj )
{
    os << &obj << '|' << obj.addedTracks << '|' << obj.addPosition << '|' << obj.removedTracks << '|' << obj.removePosition;
    return os;
}

TrackChange TrackChange::make_inverted()
{
    return TrackChange(removedTracks,removePosition,addedTracks,addPosition);
}

} // namespace