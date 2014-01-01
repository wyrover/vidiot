// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
,   mThumbnails()
{
    VAR_DEBUG(this);
}

ViewMap::~ViewMap()
{
    VAR_DEBUG(this);
    ASSERT_ZERO(mTracks.size());
    ASSERT_ZERO(mClips.size());
    ASSERT_ZERO(mThumbnails.size());
}

//////////////////////////////////////////////////////////////////////////
// REGISTRATION
//////////////////////////////////////////////////////////////////////////

void ViewMap::registerView(model::IClipPtr clip, ClipView* view)
{
    ASSERT_MAP_CONTAINS_NOT(mClips,clip);
    mClips.insert(std::make_pair(clip, view));
}

void ViewMap::registerThumbnail(model::IClipPtr clip, ThumbnailView* view)
{
    ASSERT_MAP_CONTAINS_NOT(mThumbnails,clip);
    mThumbnails.insert(std::make_pair(clip, view));
}

void ViewMap::registerView(model::TrackPtr track, TrackView* view)
{
    ASSERT_MAP_CONTAINS_NOT(mTracks,track);
    mTracks.insert(std::make_pair(track, view));
}

void ViewMap::unregisterView(model::IClipPtr clip)
{
    ASSERT_MAP_CONTAINS(mClips,clip);
    mClips.erase(clip);
}

void ViewMap::unregisterView(model::TrackPtr track)
{
    ASSERT_MAP_CONTAINS(mTracks,track);
    mTracks.erase(track);
}

void ViewMap::unregisterThumbnail(model::IClipPtr clip)
{
    ASSERT_MAP_CONTAINS(mThumbnails,clip);
    mThumbnails.erase(clip);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION
//////////////////////////////////////////////////////////////////////////

ClipView* ViewMap::getView(model::IClipPtr clip) const
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

ThumbnailView* ViewMap::getThumbnail(model::IClipPtr clip) const
{
    ThumbnailMap::const_iterator it = mThumbnails.find(clip);
    ASSERT(it != mThumbnails.end())(clip)(mThumbnails);
    return it->second;
}

}} // namespace