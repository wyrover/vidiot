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
        // Can't make the clone in the separate thread, hence this duplication.
        // Otherwise, the clip may be (partially) opened/opening in the main thread at the moment
        // the clone is made in the createBitmap method. That resulted in empty peaks views,
        // because clipclone->fileclone::mFileOpenedOk was not yet initialized.
        model::TransitionPtr inTransition{ mClip->getInTransition() };
        if (inTransition &&
            inTransition->getRight() &&
            *(inTransition->getRight()) > 0)
        {
            // This clip
            // - is part of a transition
            // - is the 'out' clip (the right one) of the transition
            // The thumbnail is the first frame after the 'logical cut' under the transitionm.
            mVideoClipClone = boost::dynamic_pointer_cast<model::VideoClip>(inTransition->makeRightClip());
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
                mVideoClipClone = boost::dynamic_pointer_cast<model::VideoClip>(outTransition->makeLeftClip());
            }
            else
            {
                mVideoClipClone = make_cloned<model::VideoClip>(boost::dynamic_pointer_cast<model::VideoClip>(mClip)); // Clone to avoid 'moving' the original clip
            }
        }
    }

    wxImagePtr createBitmap() override
    {
        // Note: if this would return a nullptr then scheduling would be repeated over and over again, since nothing is generated.
        ASSERT_MORE_THAN_EQUALS(mSize.x, 10); // Avoid issues in swscale
        ASSERT_MORE_THAN_EQUALS(mSize.y, 10); 

        if (!wxThread::IsMain())
        {
            setThreadName("RenderThumbnail");
        }

        ASSERT(mVideoClipClone);
        ASSERT(!mVideoClipClone->getTrack()); // NOTE: This is a check to ensure that a clone is used, and not the original is 'moved'

        wxImagePtr result;
        if (mVideoClipClone->getLength() > 0)
        {
            ASSERT_MORE_THAN_EQUALS_ZERO(mVideoClipClone->getOffset())(*mVideoClipClone);
            // The if is required to avoid errors during editing operations.
            mVideoClipClone->moveTo(0);
            model::VideoFramePtr videoFrame = mVideoClipClone->getNextVideo(model::VideoCompositionParameters().setBoundingBox(mSize).setDrawBoundingBox(false));
            result = videoFrame->getImage();

            // Ensure that any opened threads are closed again.
            // Avoid opening too much threads in parallel.
            mVideoClipClone->clean();
        }

        return result;
    }

    model::VideoClipPtr mVideoClipClone = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ThumbnailView::ThumbnailView(const model::IClipPtr& clip, View* parent)
:   ClipPreview(clip, parent)
{
    ASSERT(mClip->isA<model::IVideo>())(mClip);
}

//////////////////////////////////////////////////////////////////////////
// CLIPPREVIEW
//////////////////////////////////////////////////////////////////////////

RenderClipPreviewWorkPtr ThumbnailView::render() const
{
    return boost::make_shared<RenderThumbnailWork>(mClip, getSize(), getZoom().getCurrent());
}

wxSize ThumbnailView::getRequiredSize() const
{
    wxSize boundingBox{ getParent().getW() - 2 * ClipView::getBorderSize(), getParent().getH() - ClipView::getBorderSize() - ClipView::getDescriptionHeight() };
    return model::Convert::sizeInBoundingBox(model::Properties::get().getVideoSize(), boundingBox);
}

wxSize ThumbnailView::getMinimumSize() const
{
    return wxSize(10,10);
}

}} // namespace
