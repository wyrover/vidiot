#include "ThumbnailView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Clip.h"
#include "ClipView.h"
#include "Convert.h"
#include "Layout.h"
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

pixel ThumbnailView::requiredWidth() const
{
    // todo refactor this (in View class) into requiredSize method, to avoid duplicate code for x and y
    wxSize requestedSize = mVideoClip->getSize();
    wxSize boundingBox = wxSize(
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getWidth()  - 2 * Layout::sClipBorderSize,
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getHeight() - Layout::sClipBorderSize - Layout::sClipDescriptionBarHeight);
    wxSize scaledSize = model::Convert::sizeInBoundingBox(requestedSize, boundingBox);
    return scaledSize.GetWidth();
    // todo move scale algorithm ?
}

pixel ThumbnailView::requiredHeight() const
{
    wxSize requestedSize = mVideoClip->getSize();
    wxSize boundingBox = wxSize(
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getWidth()  - 2 * Layout::sClipBorderSize,
        const_cast<const ClipView*>(getViewMap().getView(mVideoClip))->getHeight() - Layout::sClipBorderSize - Layout::sClipDescriptionBarHeight);
    wxSize scaledSize = model::Convert::sizeInBoundingBox(requestedSize, boundingBox);
    return scaledSize.GetHeight();
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
    // TODO clone the clip here to avoid problems?
    mVideoClip->moveTo(0);
    model::VideoFramePtr videoFrame = mVideoClip->getNextVideo(requiredWidth(), requiredHeight(), false);
    model::wxBitmapPtr thumbnail = videoFrame->getBitmap();
    dc.DrawBitmap(*thumbnail,0,0,false);
}

}} // namespace