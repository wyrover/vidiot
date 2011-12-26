#include "ThumbnailView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Clip.h"
#include "ClipView.h"
#include "Convert.h"
#include "Layout.h"
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
    model::VideoClipPtr clone = make_cloned<model::VideoClip>(mVideoClip); // Clone to avoid 'moving' the original clip
    clone->moveTo(0);
    model::VideoFramePtr videoFrame = clone->getNextVideo(requiredSize(), false);
    model::wxBitmapPtr thumbnail = videoFrame->getBitmap();
    dc.DrawBitmap(*thumbnail,0,0,false);
}

}} // namespace