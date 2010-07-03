#include "TrackEventRemoveClips.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"
#include "Clip.h"
#include "Track.h"

wxDEFINE_EVENT(TRACK_EVENT_REMOVE_CLIPS, TrackEventRemoveClips);

TrackEventRemoveClips::TrackEventRemoveClips(model::TrackPtr track, model::Clips clips)
:   wxEvent(TRACK_EVENT_REMOVE_CLIPS)
,   mTrack(track)
,   mClips(clips)
{
    VAR_INFO(mTrack)(mClips);
}

TrackEventRemoveClips::TrackEventRemoveClips(const TrackEventRemoveClips& other)
:   wxEvent(other)
,   mTrack(other.mTrack)
,   mClips(other.mClips)
{
}

wxEvent* TrackEventRemoveClips::Clone() const 
{ 
    return new TrackEventRemoveClips(*this); 
}

model::TrackPtr TrackEventRemoveClips::getTrack() const
{
    return mTrack;
}

model::Clips TrackEventRemoveClips::getClips() const
{
    return mClips;
}
