#ifndef TRACK_EVENT_REMOVE_CLIPS_H
#define TRACK_EVENT_REMOVE_CLIPS_H

#include <wx/event.h>
#include "ModelPtr.h"

class TrackEventRemoveClips : public wxEvent
{
public:
    TrackEventRemoveClips(model::TrackPtr track, model::Clips clips);
    TrackEventRemoveClips(const TrackEventRemoveClips& other);
    model::TrackPtr getTrack() const;
    model::Clips getClips() const;
    virtual wxEvent* Clone() const;
private:
    model::TrackPtr mTrack;
    model::Clips mClips;
};

wxDECLARE_EVENT(TRACK_EVENT_REMOVE_CLIPS, TrackEventRemoveClips);

#endif TRACK_EVENT_REMOVE_CLIPS_H
