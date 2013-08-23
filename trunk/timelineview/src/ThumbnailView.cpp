// Copyright 2013 Eric Raijmakers.
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
#include "UtilCloneable.h"

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

ThumbnailView::ThumbnailView(model::IClipPtr clip, View* parent)
:   View(parent)
,   mVideoClip(boost::dynamic_pointer_cast<model::VideoClip>(clip))
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
//  GET & SET
//////////////////////////////////////////////////////////////////////////

wxSize ThumbnailView::requiredSize() const
{
    wxSize boundingBox = wxSize(
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getSize().GetWidth()  - 2 * Layout::ClipBorderSize,
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getSize().GetHeight() - Layout::ClipBorderSize - Layout::ClipDescriptionBarHeight);
    wxSize scaledSize = model::Convert::sizeInBoundingBox(model::Properties::get().getVideoSize(), boundingBox);
    scaledSize.x = std::max(model::VideoFrame::sMinimumSize, scaledSize.x); // Ensure minimum width of 10 pixels
    scaledSize.y = std::max(model::VideoFrame::sMinimumSize, scaledSize.y); // Ensure minimum height of 10 pixels
    return scaledSize;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

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
        model::VideoFramePtr videoFrame = clone->getNextVideo(model::VideoCompositionParameters().setBoundingBox(requiredSize()));
        wxBitmapPtr thumbnail = videoFrame->getBitmap();
        if (thumbnail)
        {
            dc.DrawBitmap(*thumbnail,0,0,false);
        }
    }
}

}} // namespace