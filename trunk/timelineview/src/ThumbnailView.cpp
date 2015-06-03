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

#include "ThumbnailView.h"

#include "Clip.h"
#include "ClipView.h"
#include "Convert.h"
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
    : public RenderClipPreviewWork
{
    // Here, all access to folder must be done, not in the worker thread.
    // Rationale: all access to model objects must be done in the main thread!
    explicit RenderThumbnailWork(const model::IClipPtr& clip, const wxSize& size, rational zoom)
        : RenderClipPreviewWork(clip,size,zoom)
    {
    }

    wxImagePtr createBitmap() override
    {
        if (mSize.x < 10 || mSize.y < 10) { return nullptr; } // Avoid issues in swscale

        if (!wxThread::IsMain())
        {
            setThreadName("RenderThumbnail");
        }

        model::VideoClipPtr clone;

        model::TransitionPtr inTransition{ mClip->getInTransition() };
        if (inTransition &&
            inTransition->getRight() &&
            *(inTransition->getRight()) > 0)
        {
            // This clip
            // - is part of a transition
            // - is the 'out' clip (the right one) of the transition
            // The thumbnail is the first frame after the 'logical cut' under the transitionm.
            clone = boost::dynamic_pointer_cast<model::VideoClip>(inTransition->makeRightClip());
        }
        else
        {
            model::TransitionPtr outTransition{ mClip->getOutTransition() };
            if (outTransition &&
                outTransition->getLeft() &&
                *(outTransition->getLeft()) > 0 &&
                mClip->getLength() == 0)
            {
                // This clip
                // - is part of a transition
                // - is the 'out' clip (the left one) of the transition
                // - is completely under the transition.
                // With a size 0, getting the thumbnail is impossible (since it has length 0).
                clone = boost::dynamic_pointer_cast<model::VideoClip>(outTransition->makeLeftClip());
            }
            else
            {
                clone = make_cloned<model::VideoClip>(boost::dynamic_pointer_cast<model::VideoClip>(mClip)); // Clone to avoid 'moving' the original clip
            }
        }

        ASSERT(clone);
        ASSERT(!clone->getTrack()); // NOTE: This is a check to ensure that a clone is used, and not the original is 'moved'

        wxImagePtr result;
        if (clone->getLength() > 0)
        {
            ASSERT_MORE_THAN_EQUALS_ZERO(clone->getOffset())(*clone);
            // The if is required to avoid errors during editing operations.
            clone->moveTo(0);
            model::VideoFramePtr videoFrame = clone->getNextVideo(model::VideoCompositionParameters().setBoundingBox(mSize).setDrawBoundingBox(false));
            result = videoFrame->getImage();

            // Ensure that any opened threads are closed again.
            // Avoid opening too much threads in parallel.
            clone->clean();
        }

        return result;
    }
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ThumbnailView::ThumbnailView(const model::IClipPtr& clip, View* parent)
:   ClipPreview(clip, parent)
{
}

//////////////////////////////////////////////////////////////////////////
// CLIPPREVIEW
//////////////////////////////////////////////////////////////////////////

RenderClipPreviewWorkPtr ThumbnailView::render() const
{
    return boost::make_shared<RenderThumbnailWork>(mClip, getSize(), getZoom().getCurrent());
}

wxSize ThumbnailView::requiredSize() const
{
    wxSize boundingBox{ getParent().getW() - 2 * ClipView::getBorderSize(), getParent().getH() - ClipView::getBorderSize() - ClipView::getDescriptionHeight() };
    return model::Convert::sizeInBoundingBox(model::Properties::get().getVideoSize(), boundingBox);
}

}} // namespace
