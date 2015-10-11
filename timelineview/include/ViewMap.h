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

#pragma once

#include "Part.h"

namespace gui { namespace timeline {

class DividerView;

typedef std::map< model::TrackPtr, TrackView* > TrackMap;
typedef std::map< model::TrackPtr, DividerView* > DividerMap;
typedef std::map< model::IClipPtr, ClipView* > ClipMap;
typedef std::map< model::IClipPtr, ClipPreview* > ClipPreviewMap;

class ViewMap
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ViewMap(Timeline* timeline);
    virtual ~ViewMap();

    //////////////////////////////////////////////////////////////////////////
    // REGISTRATION
    //////////////////////////////////////////////////////////////////////////

    void registerView(const model::TrackPtr& track, TrackView* view);
    void registerDivider(const model::TrackPtr& track, DividerView* view);

    void registerView(const model::IClipPtr& clip, ClipView* view);
    void registerClipPreview(const model::IClipPtr& clip, ClipPreview* view);

    void unregisterView(const model::TrackPtr &track);
    void unregisterDivider(const model::TrackPtr& track);

    void unregisterView(const model::IClipPtr& clip);
    void unregisterClipPreview(const model::IClipPtr& clip);

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION
    //////////////////////////////////////////////////////////////////////////

    virtual DividerView* getDivider(const model::TrackPtr& track) const;
    virtual TrackView* getView(const model::TrackPtr& track) const;

    virtual ClipView* getView(const model::IClipPtr& clip) const;
    virtual ClipPreview* getClipPreview(const model::IClipPtr& clip) const;

    //////////////////////////////////////////////////////////////////////////
    // MASS INVALIDATION
    //////////////////////////////////////////////////////////////////////////

    void invalidateClipPreviews();

    std::vector<ClipPreview*> getClipPreviews() const;

private:

    TrackMap mTracks;
    DividerMap mDividers;

    ClipMap mClips;
    ClipPreviewMap mClipPreviews;
};

}} // namespace
