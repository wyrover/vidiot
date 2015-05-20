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

#include "ClipPreview.h"

#include "Trim.h"
#include "Worker.h"
#include "WorkEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// RenderClipPreviewWork
//////////////////////////////////////////////////////////////////////////


RenderClipPreviewWork::RenderClipPreviewWork(const model::IClipPtr& clip, const wxSize& size, rational zoom)
    : worker::Work([this] { mResult = createBitmap(); })
    , mClip(clip)
    , mSize(size)
    , mZoom(zoom)
{
    ASSERT(mClip->getTrack()); // NOTE: This is a check to ensure that NOT a clone is stored.
}

wxSize RenderClipPreviewWork::getSize()
{
    return mSize;
}

wxBitmapPtr RenderClipPreviewWork::getResult()
{
    return mResult;
}

//////////////////////////////////////////////////////////////////////////
// COMPARISON OBJECT FOR CACHE
//////////////////////////////////////////////////////////////////////////

bool ClipPreview::CompareSize::operator()(const wxSize& s1, const wxSize& s2)
{
    return (s1.x == s2.x) ? (s1.y < s2.y) : (s1.x < s2.x);
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ClipPreview::ClipPreview(const model::IClipPtr& clip, View* parent)
:   View(parent)
,   mClip(clip)
,   mW(boost::none)
,   mH(boost::none)
,   mTrackHeight(boost::none)
{
    VAR_DEBUG(this)(mClip);
    ASSERT(mClip);

    getViewMap().registerClipPreview(mClip,this);

    // IMPORTANT: No drawing/lengthy code here. Due to the nature of adding removing clips as part of edit operations, that will severely 
    //            impact performance and may lead to crashes. As an example, consider the following clip edit operation:
    //            replace clip-transition with smallerclip-transition, then replace with smallerclip-adjustedtransition.
    //            After the first edit (smallerclip-transition) the smallerclip may be too small for the transition.
}

ClipPreview::~ClipPreview()
{
    VAR_DEBUG(this);

    getViewMap().unregisterClipPreview(mClip);
    abortPendingWork();
}

void ClipPreview::scheduleInitialRendering()
{
    if (mPendingWork.empty() && mBitmaps.empty())
    {
        scheduleRendering();
    }
}

//////////////////////////////////////////////////////////////////////////
// POSITION/SIZE
//////////////////////////////////////////////////////////////////////////

pixel ClipPreview::getX() const
{
    return getParent().getX() + Layout::ClipBorderSize;
}

pixel ClipPreview::getY() const
{
    return getParent().getY() +  Layout::get().ClipDescriptionBarHeight;
}

pixel ClipPreview::getW() const
{
    ASSERT_IMPLIES(mW,mTrackHeight);
    if (!mW || *mTrackHeight != getParent().getH())
    {
        determineSize();
    }
    return *mW;
}

pixel ClipPreview::getH() const
{
    ASSERT_IMPLIES(mH,mTrackHeight);
    if (!mH || *mTrackHeight != getParent().getH())
    {
        determineSize();
    }
    return *mH;
}

void ClipPreview::invalidateRect()
{
    mW.reset();
    mH.reset();
}

void ClipPreview::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    wxSize size(getSize());

    if (size.GetWidth() < 10 || size.GetHeight() < 10)
    {
        // if too small, then no peaks. Change track length to a very small size to test.
        // Note: this check also prevents that the peaks are drawn 'outside' the clip's region
        //       when there's just not enough room.
        return;
    }

    if (getTrim().isActive() &&
        mBitmaps.find(size) == mBitmaps.end())
    {
        // In case of trimming, update the clip preview immediately. Any newly created preview is the direct result of the trimming.
        // Do not schedule the rendering. That would cause a delay in showing the updated image.
        RenderClipPreviewWorkPtr work = render();
        work->execute(false);
        mBitmaps[size] = work->getResult();
    }

    BitmapCache::const_iterator it = mBitmaps.find(size);
    if (it == mBitmaps.end())
    {
        // Bitmap with correct size not rendered yet.

        if (mPendingWork.find(size) == mPendingWork.end())
        {
            // No work pending for this size: schedule new work
            scheduleRendering();
        }
        // else: rendering already scheduled
    }
    else
    {
        wxBitmapPtr bitmap{ it->second };
        getTimeline().copyRect(dc, region, offset, *bitmap, getRect(), bitmap->GetMask() != nullptr);
    }
}

void ClipPreview::drawForDragging(const wxPoint& position, int height, wxDC& dc) const
{
    wxSize size(0,0);
    pixel mindiff = std::numeric_limits<pixel>::max();
    wxBitmapPtr bitmap;
    for ( auto item : mBitmaps )
    {
        pixel diff = abs(height - item.first.GetHeight());
        if (diff < mindiff)
        {
            mindiff = diff;
            size = item.first;
            bitmap = item.second;
        }
    }
    if (bitmap)
    {
        wxMemoryDC dcBmp(*bitmap);
        dc.Blit(
            position.x + Layout::get().ClipBorderSize,
            position.y + Layout::get().ClipDescriptionBarHeight,
            size.GetWidth(),
            static_cast<int>(std::max(size.GetHeight(),height - Layout::get().ClipDescriptionBarHeight - Layout::ClipBorderSize )),
            &dcBmp,
            0,
            0,
            wxCOPY);
    }
}

void ClipPreview::onRenderDone(worker::WorkDoneEvent& event)
{
    RenderClipPreviewWorkPtr work = boost::dynamic_pointer_cast<RenderClipPreviewWork>(event.getValue());
    work->Unbind(worker::EVENT_WORK_DONE, &ClipPreview::onRenderDone, const_cast<ClipPreview*>(this));
    if (work->getResult())
    {
        mBitmaps[work->getSize()] = work->getResult();
    }
    mPendingWork.erase(work->getSize());
    invalidateRect();
    getTimeline().repaint(getRect());
}

//////////////////////////////////////////////////////////////////////////
// INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

void ClipPreview::invalidateCachedBitmaps()
{
    abortPendingWork();
    mBitmaps.clear();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void ClipPreview::determineSize() const
{
    static const int sMinimumSize = 10; // To avoid scaling issues with swscale
    wxSize size{ requiredSize() };
    mW.reset(std::max(sMinimumSize, size.x)); // Ensure minimum width of 10 pixels
    mH.reset(std::max(sMinimumSize, size.y)); // Ensure minimum height of 10 pixels

    // Stored to detect changes in the track height. This is done to ensure that a mismatch
    // is detected. Consider the example where a thumbnail is rendered, and then the track size
    // is reduced. Thumbnail needs to use the new track size as basis for the computation.
    mTrackHeight.reset(getParent().getH());
}

void ClipPreview::scheduleRendering() const
{
    if (!getTimeline().renderThumbnails()) { return; }
    abortPendingWork();

    RenderClipPreviewWorkPtr work = render();
    work->Bind(worker::EVENT_WORK_DONE, &ClipPreview::onRenderDone, const_cast<ClipPreview*>(this)); // No unbind: work object is destroyed when done
    worker::InvisibleWorker::get().schedule(work);
    mPendingWork[getSize()] = work;
}

void ClipPreview::abortPendingWork() const
{
    for ( auto item : mPendingWork )
    {
         boost::shared_ptr<RenderClipPreviewWork> work = boost::dynamic_pointer_cast<RenderClipPreviewWork>(item.second);
         work->abort();
         work->Unbind(worker::EVENT_WORK_DONE, &ClipPreview::onRenderDone, const_cast<ClipPreview*>(this));
    }
    mPendingWork.clear();
}

}} // namespace
