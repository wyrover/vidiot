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

#include "ThumbnailView.h"

#include "Clip.h"
#include "ClipView.h"
#include "Convert.h"
#include "Layout.h"
#include "Properties.h"
#include "Transition.h"
#include "UtilClone.h"
#include "UtilInt.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoFrame.h"
#include "VideoCompositionParameters.h"
#include "ViewMap.h"
#include "Worker.h"
#include "WorkEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// WORK OBJECT FOR RENDERING A THUMBNAIL
//////////////////////////////////////////////////////////////////////////

struct RenderThumbnailWork
    : public worker::Work
{
    // Here, all access to folder must be done, not in the worker thread.
    // Rationale: all access to model objects must be done in the main thread!
    explicit RenderThumbnailWork(const model::VideoClipPtr& clip, const wxSize& size)
        : worker::Work(boost::bind(&RenderThumbnailWork::renderThumbnail,this))
        , mClip(clip)
        , mSize(size)
    {
        ASSERT(!mClip->getTrack()); // NOTE: This is a check to ensure that a clone is used, and not the original is 'moved'
    }

    void renderThumbnail()
    {
        setThreadName("RenderThumbnail");
        wxBitmapPtr bitmap(new wxBitmap(mSize));
        if (mClip->getLength() > 0)
        {
            // The if is required to avoid errors during editing operations.
            mClip->moveTo(0); // To ensure that the VideoFile object is moved to the beginning of the clip (thus, including offset) and not the (default) beginning of the video file.
            model::VideoFramePtr videoFrame = mClip->getNextVideo(model::VideoCompositionParameters().setBoundingBox(mSize).setDrawBoundingBox(false));
            mResult = videoFrame->getBitmap();
        }
    }

    model::VideoClipPtr mClip;
    wxSize mSize;
    wxBitmapPtr mResult;
};

//////////////////////////////////////////////////////////////////////////
// COMPARISON OBJECT FOR CACHE
//////////////////////////////////////////////////////////////////////////

bool ThumbnailView::CompareSize::operator()(const wxSize& s1, const wxSize& s2)
{
    return (s1.x == s2.x) ? (s1.y < s2.y) : (s1.x < s2.x);
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ThumbnailView::ThumbnailView(const model::IClipPtr& clip, View* parent)
:   View(parent)
,   mVideoClip(boost::dynamic_pointer_cast<model::VideoClip>(clip))
,   mW(boost::none)
,   mH(boost::none)
,   mTrackHeight(boost::none)
,   mPendingWork()
,   mBitmaps()
{
    VAR_DEBUG(this)(mVideoClip);
    ASSERT(mVideoClip);

    getViewMap().registerThumbnail(mVideoClip,this);

    // IMPORTANT: No drawing/lengthy code here. Due to the nature of adding removing clips as
    //            part of edit operations, that will severely impact performance.
}

ThumbnailView::~ThumbnailView()
{
    VAR_DEBUG(this);

    getViewMap().unregisterThumbnail(mVideoClip);
    abortPendingWork();
}

void ThumbnailView::scheduleInitialRendering()
{
    if (mPendingWork.empty() && mBitmaps.empty())
    {
        scheduleRendering();
    }
}

//////////////////////////////////////////////////////////////////////////
// POSITION/SIZE
//////////////////////////////////////////////////////////////////////////

pixel ThumbnailView::getX() const
{
    return getParent().getX() + Layout::ClipBorderSize;
}

pixel ThumbnailView::getY() const
{
    return getParent().getY() +  Layout::get().ClipDescriptionBarHeight;
}

pixel ThumbnailView::getW() const
{
    ASSERT_IMPLIES(mW,mTrackHeight);
    if (!mW || *mTrackHeight != getParent().getH())
    {
        determineSize();
    }
    return *mW;
}

pixel ThumbnailView::getH() const
{
    ASSERT_IMPLIES(mH,mTrackHeight);
    if (!mH || *mTrackHeight != getParent().getH())
    {
        determineSize();
    }
    return *mH;
}

void ThumbnailView::invalidateRect()
{
    mW.reset();
    mH.reset();
}

void ThumbnailView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    wxSize size(getSize());

    if (size.GetWidth() < 10 || size.GetHeight() < 10)
    {
        // if too small, then no thumbnail. Change track length to a very small size to test.
        // Note: this check also prevents that the thumbnail is drawn 'outside' the clip's region
        //       when there's just not enough room.
        return;
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
    }
    else
    {
        getTimeline().copyRect(dc, region, offset, *(it->second), getRect());
    }
}

void ThumbnailView::drawForDragging(const wxPoint& position, int height, wxDC& dc) const
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
        wxMemoryDC dcBmp;
        dcBmp.SelectObjectAsSource(*bitmap);
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

void ThumbnailView::onRenderDone(worker::WorkDoneEvent& event)
{
    boost::shared_ptr<RenderThumbnailWork> work = boost::dynamic_pointer_cast<RenderThumbnailWork>(event.getValue());
    work->Unbind(worker::EVENT_WORK_DONE, &ThumbnailView::onRenderDone, const_cast<ThumbnailView*>(this));
    mBitmaps[work->mSize] = work->mResult;
    mPendingWork.erase(work->mSize);
    invalidateRect();
    getTimeline().repaint(getRect());
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void ThumbnailView::determineSize() const
{
    static const int sMinimumSize = 10; // To avoid scaling issues with swscale
    wxSize boundingBox = wxSize( getParent().getW() - 2 * Layout::ClipBorderSize, getParent().getH() - Layout::ClipBorderSize - Layout::ClipDescriptionBarHeight);
    wxSize scaledSize = model::Convert::sizeInBoundingBox(model::Properties::get().getVideoSize(), boundingBox);
    mW.reset(std::max(sMinimumSize, scaledSize.x)); // Ensure minimum width of 10 pixels
    mH.reset(std::max(sMinimumSize, scaledSize.y)); // Ensure minimum height of 10 pixels

    // Stored to detect changes in the track height. This is done to ensure that a mismatch
    // is detected. Consider the example where a thumbnail is rendered, and then the track size
    // is reduced. Thumbnail needs to use the new track size as basis for the computation.
    mTrackHeight.reset(getParent().getH());
}

model::VideoClipPtr ThumbnailView::getClip() const
{
    model::VideoClipPtr clone;

    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mVideoClip->getPrev());
    if (transition && 
        transition->getRight() && 
        *(transition->getRight()) > 0)
    {
        // This clip
        // - is part of a transition
        // - is the 'out' clip (the right one) of the transition
        // The thumbnail is the first frame after the 'logical cut' under the transitionm.
        clone = boost::dynamic_pointer_cast<model::VideoClip>(transition->makeRightClip());
    }
    else
    {
        model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mVideoClip->getNext());
        if (transition && 
            transition->getLeft() &&
            *(transition->getLeft()) > 0 && 
            mVideoClip->getLength() == 0)
        {
            // This clip
            // - is part of a transition
            // - is the 'out' clip (the left one) of the transition
            // - is completely under the transition.
            // With a size 0, getting the thumbnail is impossible (since it has length 0).
            clone = boost::dynamic_pointer_cast<model::VideoClip>(transition->makeLeftClip());
        }
        else
        {
            clone = make_cloned<model::VideoClip>(mVideoClip); // Clone to avoid 'moving' the original clip
        }
    }
    ASSERT(clone);
    ASSERT(!clone->getTrack()); // NOTE: This is a check to ensure that a clone is used, and not the original is 'moved'
    return clone;
}

void ThumbnailView::scheduleRendering() const
{
    if (!getTimeline().renderThumbnails()) { return; }
    abortPendingWork();

    wxSize size(getSize());
    boost::shared_ptr<RenderThumbnailWork> work = boost::make_shared<RenderThumbnailWork>(getClip(),size);
    work->Bind(worker::EVENT_WORK_DONE, &ThumbnailView::onRenderDone, const_cast<ThumbnailView*>(this)); // No unbind: work object is destroyed when done
    worker::InvisibleWorker::get().schedule(work);
    mPendingWork[size] = work;
}

void ThumbnailView::abortPendingWork() const
{
    for ( auto item : mPendingWork )
    {
         boost::shared_ptr<RenderThumbnailWork> work = boost::dynamic_pointer_cast<RenderThumbnailWork>(item.second);
         work->abort();
         work->Unbind(worker::EVENT_WORK_DONE, &ThumbnailView::onRenderDone, const_cast<ThumbnailView*>(this));
    }
    mPendingWork.clear();
}

}} // namespace