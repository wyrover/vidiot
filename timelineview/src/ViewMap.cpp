// Copyright 2013-2015 Eric Raijmakers.
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

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/adaptor/map.hpp>

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ViewMap::ViewMap(Timeline* timeline)
    : Part(timeline)
    , mTracks()
    , mDividers()
    , mClips()
    , mClipPreviews()
{
    VAR_DEBUG(this);
}

ViewMap::~ViewMap()
{
    VAR_DEBUG(this);
    ASSERT_ZERO(mTracks.size());
    ASSERT_ZERO(mDividers.size());
    ASSERT_ZERO(mClips.size());
    ASSERT_ZERO(mClipPreviews.size());
}

//////////////////////////////////////////////////////////////////////////
// REGISTRATION
//////////////////////////////////////////////////////////////////////////

void ViewMap::registerView(const model::TrackPtr& track, TrackView* view)
{
    ASSERT_MAP_CONTAINS_NOT(mTracks,track);
    mTracks.insert(std::make_pair(track, view));
}

void ViewMap::registerDivider(const model::TrackPtr& track, DividerView* view)
{
    ASSERT_MAP_CONTAINS_NOT(mDividers,track);
    mDividers.insert(std::make_pair(track, view));
}

void ViewMap::registerView(const model::IClipPtr &clip, ClipView* view)
{
    ASSERT_MAP_CONTAINS_NOT(mClips,clip);
    mClips.insert(std::make_pair(clip, view));
}

void ViewMap::registerClipPreview(const model::IClipPtr& clip, ClipPreview* view)
{
    ASSERT_MAP_CONTAINS_NOT(mClipPreviews,clip);
    mClipPreviews.insert(std::make_pair(clip, view));
}

void ViewMap::unregisterView(const model::TrackPtr& track)
{
    ASSERT_MAP_CONTAINS(mTracks,track);
    mTracks.erase(track);
}

void ViewMap::unregisterDivider(const model::TrackPtr& track)
{
    ASSERT_MAP_CONTAINS(mDividers,track);
    mDividers.erase(track);
}

void ViewMap::unregisterView(const model::IClipPtr& clip)
{
    ASSERT_MAP_CONTAINS(mClips,clip);
    mClips.erase(clip);
}

void ViewMap::unregisterClipPreview(const model::IClipPtr& clip)
{
    ASSERT_MAP_CONTAINS(mClipPreviews,clip);
    mClipPreviews.erase(clip);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION
//////////////////////////////////////////////////////////////////////////

TrackView* ViewMap::getView(const model::TrackPtr& track) const
{
    TrackMap::const_iterator it = mTracks.find(track);
    ASSERT(it != mTracks.end())(track)(mTracks);
    return it->second;
}

DividerView* ViewMap::getDivider(const model::TrackPtr& track) const
{
    DividerMap::const_iterator it = mDividers.find(track);
    ASSERT(it != mDividers.end())(track)(mDividers);
    return it->second;
}

ClipView* ViewMap::getView(const model::IClipPtr& clip) const
{
    ClipMap::const_iterator it = mClips.find(clip);
    ASSERT(it != mClips.end())(clip)(mClips);
    return it->second;
}

ClipPreview* ViewMap::getClipPreview(const model::IClipPtr& clip) const
{
    ClipPreviewMap::const_iterator it = mClipPreviews.find(clip);
    ASSERT(it != mClipPreviews.end())(clip)(mClipPreviews);
    return it->second;
}

//////////////////////////////////////////////////////////////////////////
// MASS INVALIDATION
//////////////////////////////////////////////////////////////////////////

void ViewMap::invalidateClipPreviews()
{
    for (auto v : mClipPreviews )
    {
        v.second->invalidateRect();
    }
}

std::vector<ClipPreview*> ViewMap::getClipPreviews() const
{
    std::vector<ClipPreview*> result;
    boost::copy(mClipPreviews | boost::adaptors::map_values, std::back_inserter(result));
    return result;
}

}} // namespace