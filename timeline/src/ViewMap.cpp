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
//
//////////////////////////////////////////////////////////////////////////

void ViewMap::add(model::ClipPtr modelClip, GuiTimeLineClipPtr clipView)
{
    mClips.insert(ClipMap::value_type(modelClip, clipView));
}

void ViewMap::add(model::TrackPtr modelTrack, GuiTimeLineTrackPtr trackView)
{
    mTracks.insert(TrackMap::value_type(modelTrack, trackView));
}

void ViewMap::remove(model::ClipPtr modelClip)
{
    mClips.left.erase(modelClip);
}

void ViewMap::remove(model::TrackPtr modelTrack)
{
    mTracks.left.erase(modelTrack);
}

void ViewMap::remove(GuiTimeLineClipPtr clipView)
{
    mClips.right.erase(clipView);
}

void ViewMap::remove(GuiTimeLineTrackPtr trackView)
{
    mTracks.right.erase(trackView);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

GuiTimeLineClipPtr ViewMap::ModelToView(model::ClipPtr modelClip)
{
    ClipMap::left_const_iterator it = mClips.left.find(modelClip);
    ASSERT(it != mClips.left.end());
    return it->second;
}

GuiTimeLineTrackPtr ViewMap::ModelToView(model::TrackPtr modelTrack)
{
    TrackMap::left_const_iterator it = mTracks.left.find(modelTrack);
    ASSERT(it != mTracks.left.end());
    return it->second;
}

model::ClipPtr ViewMap::ViewToModel(GuiTimeLineClipPtr viewClip)
{
    ClipMap::right_const_iterator it = mClips.right.find(viewClip);
    ASSERT(it != mClips.right.end());
    return it->second;
}

model::TrackPtr ViewMap::ViewToModel(GuiTimeLineTrackPtr viewTrack)
{
    TrackMap::right_const_iterator it = mTracks.right.find(viewTrack);
    ASSERT(it != mTracks.right.end());
    return it->second;
}

}} // namespace
