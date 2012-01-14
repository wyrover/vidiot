#include "ThumbnailView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Clip.h"
#include "ClipView.h"
#include "Convert.h"
#include "Layout.h"
#include "Transition.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoFrame.h"
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
    wxSize requestedSize = mVideoClip->getSize();
    wxSize boundingBox = wxSize(
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getSize().GetWidth()  - 2 * Layout::sClipBorderSize,
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getSize().GetHeight() - Layout::sClipBorderSize - Layout::sClipDescriptionBarHeight);
    wxSize scaledSize = model::Convert::sizeInBoundingBox(requestedSize, boundingBox);
    return scaledSize;
}

void ThumbnailView::redraw()
{
    invalidateBitmap();
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
    clone->moveTo(0);
    model::VideoFramePtr videoFrame = clone->getNextVideo(requiredSize(), false);
    model::wxBitmapPtr thumbnail = videoFrame->getBitmap();
    dc.DrawBitmap(*thumbnail,0,0,false);
}

}} // namespace