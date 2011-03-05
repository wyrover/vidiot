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
    VAR_DEBUG(this);
}

ViewMap::~ViewMap()
{
    VAR_DEBUG(this);
    ASSERT(mTracks.size() == 0)(mTracks);
    ASSERT(mClips.size() == 0)(mClips);
}

//////////////////////////////////////////////////////////////////////////
// REGISTRATION
//////////////////////////////////////////////////////////////////////////

void ViewMap::registerView(model::ClipPtr clip, ClipView* view)
{
    ASSERT(mClips.find(clip) == mClips.end());
    mClips.insert(std::make_pair(clip, view));
}

void ViewMap::registerView(model::TrackPtr track, TrackView* view)
{
    ASSERT(mTracks.find(track) == mTracks.end());
    mTracks.insert(std::make_pair(track, view));
}

void ViewMap::unregisterView(model::ClipPtr clip)
{
    ASSERT(mClips.find(clip) != mClips.end());
    mClips.erase(clip);
}

void ViewMap::unregisterView(model::TrackPtr track)
{
    ASSERT(mTracks.find(track) != mTracks.end());
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
