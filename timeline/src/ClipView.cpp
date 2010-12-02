#include "ClipView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Zoom.h"
#include "Constants.h"
#include "UtilLog.h"
#include "VideoFrame.h"
#include "VideoClip.h"
#include "EmptyClip.h"
#include "Selection.h"
#include "ViewMap.h"
#include "Track.h"
#include "Clip.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ClipView::ClipView(model::ClipPtr clip, View* parent)
:   View(parent)
,   mClip(clip)
,   mThumbnail()
,   mRect(0,0,0,0)
{
    ASSERT(mClip);
    getViewMap().registerView(mClip,this);
    updateThumbnail();
}

ClipView::~ClipView()
{
    getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
//  GET & SET
//////////////////////////////////////////////////////////////////////////

model::ClipPtr ClipView::getClip()
{
    return mClip;
}

boost::int64_t ClipView::getLeftPosition() const
{
    return getZoom().ptsToPixels(mClip->getLeftPts());
}

boost::int64_t ClipView::getRightPosition() const
{
    return getZoom().ptsToPixels(mClip->getRightPts());
}

void ClipView::show(wxRect rect)
{
    mRect.width = rect.width;
    mRect.x = rect.x;
    mRect.y = 4;
    mRect.height = requiredHeight() - 8; 
    invalidateBitmap();;
}

int ClipView::requiredWidth()
{
    return getRightPosition() - getLeftPosition();
}

int ClipView::requiredHeight()
{
    return mClip->getTrack()->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void ClipView::updateThumbnail()
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(mClip);
    if (videoclip)
    {
        mClip->moveTo(0);
        model::VideoFramePtr videoFrame = videoclip->getNextVideo(requiredWidth() - 2 * Constants::sClipBorderSize, requiredHeight() - 2 * Constants::sClipBorderSize, false);
        mThumbnail.reset(new wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
        mClip->moveTo(0);
    }
    invalidateBitmap();
}

void ClipView::draw(wxBitmap& bitmap)
{
    wxMemoryDC dc(bitmap);

    if (mClip->isA<model::EmptyClip>())
    {
        dc.SetBrush(Constants::sBackgroundBrush);
        dc.SetPen(Constants::sBackgroundPen);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    }
    else
    {
        if (getSelection().isSelected(mClip))
        {
            dc.SetBrush(Constants::sSelectedClipBrush);
            dc.SetPen(Constants::sSelectedClipPen);
        }
        else
        {
            dc.SetBrush(Constants::sClipBrush);
            dc.SetPen(Constants::sClipPen);
        }
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
        if (mThumbnail)
        {
            dc.DrawBitmap(*mThumbnail,wxPoint(Constants::sClipBorderSize,Constants::sClipBorderSize));
        }
    }

    if (mRect.GetHeight() != 0)
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawRectangle(mRect);
    }
}

}} // namespace
