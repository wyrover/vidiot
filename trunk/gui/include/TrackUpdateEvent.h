#ifndef GUI_TRACK_UPDATE_EVENT_H
#define GUI_TRACK_UPDATE_EVENT_H

#include <wx/event.h>
#include "GuiPtr.h"

class TrackUpdateEvent 
    :   public wxEvent
{
public:
    TrackUpdateEvent(GuiTimeLineTrackPtr track);
    TrackUpdateEvent(const TrackUpdateEvent& other);
    virtual wxEvent* Clone() const;
    GuiTimeLineTrackPtr track();
private:
    GuiTimeLineTrackPtr mTrack;
};

wxDECLARE_EVENT(TRACK_UPDATE_EVENT, TrackUpdateEvent);

#endif // GUI_TRACK_UPDATE_EVENT_H