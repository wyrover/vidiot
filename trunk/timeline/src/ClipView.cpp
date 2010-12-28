#include "ClipView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Zoom.h"
#include "Layout.h"
#include "Drag.h"
#include "UtilLog.h"
#include "VideoFrame.h"
#include "VideoClip.h"
#include "EmptyClip.h"
#include "Selection.h"
#include "ViewMap.h"
#include "Track.h"
#include "Clip.h"
#include "PositionInfo.h"

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
    mClip->Bind(model::EVENT_SELECT_CLIP,   &ClipView::onClipSelected,  this);
    updateThumbnail();
}

ClipView::~ClipView()
{
    mClip->Unbind(model::EVENT_SELECT_CLIP, &ClipView::onClipSelected,  this);
    getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
//  GET & SET
//////////////////////////////////////////////////////////////////////////

model::ClipPtr ClipView::getClip()
{
    return mClip;
}

pixel ClipView::getLeftPosition() const
{
    return getZoom().ptsToPixels(mClip->getLeftPts());
}

pixel ClipView::getRightPosition() const
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

pixel ClipView::requiredWidth() const
{
    return getRightPosition() - getLeftPosition();
}

pixel ClipView::requiredHeight() const
{
    return mClip->getTrack()->getHeight();
}

void ClipView::getPositionInfo(wxPoint position, PointerPositionInfo& info) const
{
    ASSERT(info.track); // If the track is not filled in, then how can this clipview be reached?

    // This is handled on a per-pixel and not per-pts basis. That ensures
    // that this still works for clips which are very small when zoomed out.
    // (then the cursor won't flip too much).
    int dist_begin = position.x - getLeftPosition();
    int dist_end = getRightPosition() - position.x;

    if (dist_begin <= 1)
    {
        // Possibly between clips. However, this is only relevant if there
        // is another nonempty clip adjacent to this clip.
        model::ClipPtr previous = info.track->getPreviousClip(info.clip);
        info.logicalclipposition = (!previous || previous->isA<model::EmptyClip>()) ? ClipBegin : ClipBetween;
    }
    else if (dist_end <= 1)
    {
        // Possibly between clips. However, this is only relevant if there
        // is another nonempty clip adjacent to this clip.
        model::ClipPtr next = info.track->getNextClip(info.clip);
        info.logicalclipposition = (!next || next->isA<model::EmptyClip>()) ? ClipEnd : ClipBetween;
    }
    else if ((dist_begin > 1) && (dist_begin < 4))
    {
        info.logicalclipposition = ClipBegin;
    }
    else if ((dist_end > 1) && (dist_end < 4))
    {
        info.logicalclipposition = ClipEnd;
    }
    else
    {
        info.logicalclipposition = ClipInterior;
    }
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
        model::VideoFramePtr videoFrame = videoclip->getNextVideo(requiredWidth() - 2 * Layout::sClipBorderSize, requiredHeight() - 2 * Layout::sClipBorderSize, false);
        mThumbnail.reset(new wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
        mClip->moveTo(0);
    }
    invalidateBitmap();
}

void ClipView::draw(wxBitmap& bitmap) const
{
    draw(bitmap, !getDrag().isActive(), true);
}

void ClipView::draw(wxBitmap& bitmap, bool drawSelectedClips, bool drawUnselectedClips) const
{
    // NOTE: DO NOT use requiredWidth/requiredHeight here, since
    //       This method is also used for drawing clips that are
    //       dragged around, and thus can have a different size.
    //       (caused by hovering such a clip over another track
    //       typically causes it to be drawn with a different
    //       height).

    wxMemoryDC dc(bitmap);

    if (mClip->isA<model::EmptyClip>() || 
        (!drawSelectedClips && mClip->getSelected()) ||
        (!drawUnselectedClips && !mClip->getSelected()))
    {
        // For empty clips, the bitmap is empty.
        // Selected clips that are being dragged should no longer be drawn
        // in the regular tracks as they have become part of 'getDrag()'s bitmap.
        dc.SetBrush(Layout::sBackgroundBrush);
        dc.SetPen(Layout::sBackgroundPen);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    }
    else
    {
        if (mClip->getSelected())
        {
            dc.SetBrush(Layout::sSelectedClipBrush);
            dc.SetPen(Layout::sSelectedClipPen);
        }
        else
        {
            dc.SetBrush(Layout::sClipBrush);
            dc.SetPen(Layout::sClipPen);
        }
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
        if (mThumbnail)
        {
            dc.DrawBitmap(*mThumbnail,wxPoint(Layout::sClipBorderSize,Layout::sClipBorderSize));
        }
    }

    if (mRect.GetHeight() != 0)
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawRectangle(mRect);
    }
}

void ClipView::drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const
{
    if (mClip->getSelected())
    {
        wxBitmap b(requiredWidth(), height);
        draw(b, true, false);
        dc.DrawBitmap(b,position);
        dcMask.DrawRectangle(position,b.GetSize());
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void ClipView::onClipSelected( model::EventSelectClip& event )
{
    invalidateBitmap();
    event.Skip();
}

}} // namespace
