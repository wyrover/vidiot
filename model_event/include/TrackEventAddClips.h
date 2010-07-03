#ifndef TRACK_EVENT_ADD_CLIPS_H
#define TRACK_EVENT_ADD_CLIPS_H

#include <wx/event.h>
#include "ModelPtr.h"

class TrackEventAddClips : public wxEvent
{
public:
    TrackEventAddClips(model::TrackPtr track, model::Clips clips, model::ClipPtr position);
    TrackEventAddClips(const TrackEventAddClips& other);
    model::TrackPtr getTrack() const;
    model::Clips getClips() const;
    
    /**
    * Clips are be inserted BEFORE this position. 
    * If clips are inserted at the end of the track this is
    * an uninitialized pointer.
    */
    model::ClipPtr getPosition();
    virtual wxEvent* Clone() const;
private:
    model::TrackPtr mTrack;
    model::Clips mClips;
    model::ClipPtr mPosition;
};

wxDECLARE_EVENT(TRACK_EVENT_ADD_CLIPS, TrackEventAddClips);

#endif TRACK_EVENT_ADD_CLIPS_H
