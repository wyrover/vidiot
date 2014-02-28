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
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ThumbnailView::ThumbnailView(const model::IClipPtr& clip, View* parent)
:   View(parent)
,   mVideoClip(boost::dynamic_pointer_cast<model::VideoClip>(clip))
,   mW(boost::none)
,   mH(boost::none)
,   mBitmap(boost::none)
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
    if (!mW)
    {
        determineSize();
    }
    return *mW;
}

pixel ThumbnailView::getH() const
{
    if (!mH)
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
    if (!mBitmap || mBitmap->GetSize() != size)
    {
        if (size.GetWidth() >= 10 && size.GetHeight() >= 10) // if too small, then no thumbnail
        {
            mBitmap.reset(wxBitmap(size));
            draw(*mBitmap);
        }
    }
    if (mBitmap)
    {
        getTimeline().copyRect(dc, region, offset, *mBitmap, getRect());
    }
}

void ThumbnailView::drawForDragging(const wxPoint& position, int height, wxDC& dc) const
{
    if (mBitmap)
    {
        wxMemoryDC dcBmp;
        dcBmp.SelectObjectAsSource(*mBitmap);
        dc.Blit(
            position.x + Layout::get().ClipBorderSize,
            position.y + Layout::get().ClipDescriptionBarHeight,
            getW(),
            static_cast<int>(std::max(getH(),height - Layout::get().ClipDescriptionBarHeight - Layout::ClipBorderSize )),
            &dcBmp,
            0,
            0,
            wxCOPY);
    }
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
}

void ThumbnailView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    model::VideoClipPtr clone;

    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mVideoClip->getPrev());
    if (transition && transition->getRight() > 0)
    {
        // This clip
        // - is part of a transition
        // - is the 'in' clip (the right one) of the transition
        // The thumbnail is the first frame after the 'logical cut' under the transitionm.
        clone = boost::dynamic_pointer_cast<model::VideoClip>(transition->makeRightClip());
    }
    else
    {
        model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mVideoClip->getNext());
        if (transition && transition->getLeft() > 0 && mVideoClip->getLength() == 0)
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
    if (clone->getLength() > 0)
    {
        // The if is required to avoid errors during editing operations.
        clone->moveTo(0); // To ensure that the VideoFile object is moved to the beginning of the clip (thus, including offset) and not the (default) beginning of the video file.
        model::VideoFramePtr videoFrame = clone->getNextVideo(model::VideoCompositionParameters().setBoundingBox(getSize()).setDrawBoundingBox(false));
        wxBitmapPtr thumbnail = videoFrame->getBitmap();
        if (thumbnail)
        {
            dc.DrawBitmap(*thumbnail,0,0,false);
        }
    }
}

}} // namespace