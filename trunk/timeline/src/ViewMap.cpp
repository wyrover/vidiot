#include "ViewMap.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ViewMap::ViewMap()
:   mTracks()
,   mClips()
{
}

ViewMap::~ViewMap()
{
}

//////////////////////////////////////////////////////////////////////////
// REGISTRATION
//////////////////////////////////////////////////////////////////////////

void ViewMap::registerView(model::ClipPtr clip, GuiTimeLineClip* view)
{
    mClips.insert(std::make_pair(clip, view));
}

void ViewMap::registerView(model::TrackPtr track, GuiTimeLineTrack* view)
{
    mTracks.insert(std::make_pair(track, view));
}

void ViewMap::unregisterView(model::ClipPtr clip)
{
    mClips.erase(clip);
}

void ViewMap::unregisterView(model::TrackPtr track)
{
    mTracks.erase(track);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION
//////////////////////////////////////////////////////////////////////////

GuiTimeLineClip* ViewMap::getView(model::ClipPtr clip) const
{
    ClipMap::const_iterator it = mClips.find(clip);
    ASSERT(it != mClips.end());
    return it->second;
}

GuiTimeLineTrack* ViewMap::getView(model::TrackPtr track) const
{
    TrackMap::const_iterator it = mTracks.find(track);
    ASSERT(it != mTracks.end());
    return it->second;
}

}} // namespace
