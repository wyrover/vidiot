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

#ifndef VIEW_MAP_H
#define VIEW_MAP_H

#include "Part.h"

namespace gui { namespace timeline {

class DividerView;

typedef std::map< model::TrackPtr, TrackView* > TrackMap;
typedef std::map< model::TrackPtr, DividerView* > DividerMap;
typedef std::map< model::IClipPtr, ClipView* > ClipMap;
typedef std::map< model::IClipPtr, ThumbnailView* > ThumbnailMap;

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
    void registerThumbnail(const model::IClipPtr& clip, ThumbnailView* view);

    void unregisterView(const model::TrackPtr &track);
    void unregisterDivider(const model::TrackPtr& track);

    void unregisterView(const model::IClipPtr& clip);
    void unregisterThumbnail(const model::IClipPtr& clip);

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION
    //////////////////////////////////////////////////////////////////////////

    virtual DividerView* getDivider(const model::TrackPtr& track) const;
    virtual TrackView* getView(const model::TrackPtr& track) const;

    virtual ClipView* getView(const model::IClipPtr& clip) const;
    virtual ThumbnailView* getThumbnail(const model::IClipPtr& clip) const;

    //////////////////////////////////////////////////////////////////////////
    // MASS INVALIDATION
    //////////////////////////////////////////////////////////////////////////

    void invalidateThumbnails();

    std::list<ThumbnailView*> getThumbnails() const;

private:

    TrackMap mTracks;
    DividerMap mDividers;

    ClipMap mClips;
    ThumbnailMap mThumbnails;
};

}} // namespace

#endif
