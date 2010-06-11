#include "TrackUpdateEvent.h"

wxDEFINE_EVENT(TRACK_UPDATE_EVENT, TrackUpdateEvent);

TrackUpdateEvent::TrackUpdateEvent(GuiTimeLineTrackPtr track)
:   wxEvent(wxID_ANY,TRACK_UPDATE_EVENT)
,   mTrack(track)
{
}

TrackUpdateEvent::TrackUpdateEvent(const TrackUpdateEvent& other)
:   wxEvent(other)
,   mTrack(other.mTrack)
{
}

wxEvent* TrackUpdateEvent::Clone() const
{
    return new TrackUpdateEvent(*this);
}

GuiTimeLineTrackPtr TrackUpdateEvent::track()
{
    return mTrack;
}
