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

#ifndef THUMBNAIL_VIEW_H
#define THUMBNAIL_VIEW_H

#include "View.h"

namespace worker {
    class WorkDoneEvent;
}

namespace gui { namespace timeline {

class ThumbnailView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ThumbnailView(const model::IClipPtr& clip, View* parent);
    virtual ~ThumbnailView();

    void scheduleInitialRendering();

    //////////////////////////////////////////////////////////////////////////
    // VIEW
    //////////////////////////////////////////////////////////////////////////

    pixel getX() const override;
    pixel getY() const override;
    pixel getW() const override;
    pixel getH() const override;

    void invalidateRect() override;

    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const override;
    void drawForDragging(const wxPoint& position, int height, wxDC& dc) const;

    void onRenderDone(worker::WorkDoneEvent& event);

private:

    struct CompareSize
    {
         bool operator()(const wxSize& s1, const wxSize& s2);
    };

    model::VideoClipPtr mVideoClip;
    mutable boost::optional<pixel> mW;
    mutable boost::optional<pixel> mH;
    mutable boost::optional<pixel> mTrackHeight;
    typedef std::map<wxSize, worker::WorkPtr, CompareSize> PendingWork;
    mutable PendingWork mPendingWork;
    typedef std::map<wxSize, wxBitmapPtr, CompareSize> BitmapCache;
    mutable BitmapCache mBitmaps;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void determineSize() const;

    model::VideoClipPtr getClip() const;

    void scheduleRendering() const;
    void abortPendingWork() const;

};

}} // namespace

#endif
