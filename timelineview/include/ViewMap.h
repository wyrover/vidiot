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

    void registerView(model::TrackPtr track, TrackView* view);
    void registerDivider(model::TrackPtr track, DividerView* view);

    void registerView(model::IClipPtr clip, ClipView* view);
    void registerThumbnail(model::IClipPtr clip, ThumbnailView* view);

    void unregisterView(model::TrackPtr track);
    void unregisterDivider(model::TrackPtr track);

    void unregisterView(model::IClipPtr clip);
    void unregisterThumbnail(model::IClipPtr clip);

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION
    //////////////////////////////////////////////////////////////////////////

    virtual DividerView* getDivider(model::TrackPtr track) const;
    virtual TrackView* getView(model::TrackPtr track) const;

    virtual ClipView* getView(model::IClipPtr clip) const;
    virtual ThumbnailView* getThumbnail(model::IClipPtr clip) const;

    //////////////////////////////////////////////////////////////////////////
    // MASS INVALIDATION
    //////////////////////////////////////////////////////////////////////////

    void invalidateThumbnails();

private:

    TrackMap mTracks;
    DividerMap mDividers;

    ClipMap mClips;
    ThumbnailMap mThumbnails;
};

}} // namespace

#endif // VIEW_MAP_H