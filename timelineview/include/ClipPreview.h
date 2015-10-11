// Copyright 2015 Eric Raijmakers.
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

#include "View.h"
#include "Work.h"

namespace worker {
    class WorkDoneEvent;
}

namespace gui { namespace timeline {

class RenderClipPreviewWork
    : public worker::Work
{
public:

    explicit RenderClipPreviewWork(const model::IClipPtr& clip, const wxSize& size, rational zoom);

    wxImagePtr getResult();
    wxSize getSize();

protected:

    virtual wxImagePtr createBitmap() = 0;

    const model::IClipPtr mClip;
    const wxSize mSize;
    const rational mZoom;

private:

    wxImagePtr mResult;
};

typedef boost::shared_ptr<RenderClipPreviewWork> RenderClipPreviewWorkPtr;

class ClipPreview
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ClipPreview(const model::IClipPtr& clip, View* parent);
    virtual ~ClipPreview();

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

    void invalidateCachedBitmaps();

protected:

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE TO SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    virtual RenderClipPreviewWorkPtr render() const = 0;

    virtual wxSize getRequiredSize() const = 0;
    virtual wxSize getMinimumSize() const = 0;

    model::IClipPtr mClip;

private:

    struct CompareSize
    {
         bool operator()(const wxSize& s1, const wxSize& s2);
    };

    mutable boost::optional<pixel> mW;
    mutable boost::optional<pixel> mH;
    mutable boost::optional<pixel> mTrackHeight;
    typedef std::map<wxSize, RenderClipPreviewWorkPtr, CompareSize> PendingWork;
    mutable PendingWork mPendingWork;
    typedef std::map<wxSize, wxImagePtr, CompareSize> ImageCache;
    mutable ImageCache mImages;
    typedef std::map<wxSize, wxBitmapPtr, CompareSize> BitmapCache;
    mutable BitmapCache mBitmaps;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void determineSize() const;

    void scheduleRendering() const;
    void abortPendingWork() const;

    wxBitmapPtr getCachedBitmap(wxSize size) const;
};

}} // namespace
