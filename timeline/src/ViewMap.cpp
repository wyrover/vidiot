#include "ViewMap.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ViewMap::ViewMap(Timeline* timeline)
:   Part(timeline)
,   mTracks()
,   mClips()
{
}

ViewMap::~ViewMap()
{
}

//////////////////////////////////////////////////////////////////////////
// REGISTRATION
//////////////////////////////////////////////////////////////////////////

void ViewMap::registerView(model::ClipPtr clip, ClipView* view)
{
    mClips.insert(std::make_pair(clip, view));
}

void ViewMap::registerView(model::TrackPtr track, TrackView* view)
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

ClipView* ViewMap::getView(model::ClipPtr clip) const
{
    ClipMap::const_iterator it = mClips.find(clip);
    ASSERT(it != mClips.end())(clip)(mClips);
    return it->second;
}

TrackView* ViewMap::getView(model::TrackPtr track) const
{
    TrackMap::const_iterator it = mTracks.find(track);
    ASSERT(it != mTracks.end())(track)(mTracks);
    return it->second;
}

}} // namespace
