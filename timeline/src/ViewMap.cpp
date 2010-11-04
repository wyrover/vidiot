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

void ViewMap::add(model::ClipPtr modelClip, GuiTimeLineClipPtr clipView)
{
    mClips.insert(std::make_pair(modelClip, clipView));
}

void ViewMap::add(model::TrackPtr modelTrack, GuiTimeLineTrackPtr trackView)
{
    mTracks.insert(std::make_pair(modelTrack, trackView));
}

void ViewMap::remove(model::ClipPtr modelClip)
{
    mClips.erase(modelClip);
}

void ViewMap::remove(model::TrackPtr modelTrack)
{
    mTracks.erase(modelTrack);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION
//////////////////////////////////////////////////////////////////////////

GuiTimeLineClipPtr ViewMap::ModelToView(model::ClipPtr modelClip) const
{
    ClipMap::const_iterator it = mClips.find(modelClip);
    ASSERT(it != mClips.end());
    return it->second;
}

GuiTimeLineTrackPtr ViewMap::ModelToView(model::TrackPtr modelTrack) const
{
    TrackMap::const_iterator it = mTracks.find(modelTrack);
    ASSERT(it != mTracks.end());
    return it->second;
}

}} // namespace
