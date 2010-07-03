#include "TrackEventAddClips.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"
#include "Clip.h"
#include "Track.h"

wxDEFINE_EVENT(TRACK_EVENT_ADD_CLIPS, TrackEventAddClips);

TrackEventAddClips::TrackEventAddClips(model::TrackPtr track, model::Clips clips, model::ClipPtr position)
:   wxEvent(TRACK_EVENT_ADD_CLIPS)
,   mTrack(track)
,   mClips(clips)
,   mPosition(position)
{
    VAR_INFO(mTrack)(mClips)(mPosition);
}

TrackEventAddClips::TrackEventAddClips(const TrackEventAddClips& other)
:   wxEvent(other)
,   mTrack(other.mTrack)
,   mClips(other.mClips)
,   mPosition(other.mPosition)
{
}

wxEvent* TrackEventAddClips::Clone() const 
{ 
    return new TrackEventAddClips(*this); 
}

model::TrackPtr TrackEventAddClips::getTrack() const
{
    return mTrack;
}

model::Clips TrackEventAddClips::getClips() const
{
    return mClips;
}

model::ClipPtr TrackEventAddClips::getPosition()
{
    return mPosition;
}
