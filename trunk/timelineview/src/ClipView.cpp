#include "ClipView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Config.h"
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
#include "Options.h"
#include "Transition.h"
#include "Clip.h"
#include "PositionInfo.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ClipView::ClipView(model::IClipPtr clip, View* parent)
:   View(parent)
,   mClip(clip)
,   mThumbnail()
,   mRect(0,0,0,0)
,   mBeginAddition(0)
{
    VAR_DEBUG(this)(mClip);
    ASSERT(mClip);

    getViewMap().registerView(mClip,this);
    mClip->Bind(model::EVENT_DRAG_CLIP,             &ClipView::onClipDragged,           this);
    mClip->Bind(model::EVENT_SELECT_CLIP,           &ClipView::onClipSelected,          this);
    mClip->Bind(model::DEBUG_EVENT_RENDER_PROGRESS, &ClipView::onGenerationProgress,    this);
    // todo also handle these events for transitions
    updateThumbnail();
}

ClipView::~ClipView()
{
    VAR_DEBUG(this);

    mClip->Unbind(model::EVENT_DRAG_CLIP,             &ClipView::onClipDragged,         this);
    mClip->Unbind(model::EVENT_SELECT_CLIP,           &ClipView::onClipSelected,        this);
    mClip->Unbind(model::DEBUG_EVENT_RENDER_PROGRESS, &ClipView::onGenerationProgress,  this);
    
    getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
//  GET & SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr ClipView::getClip()
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
    mRect.height = getHeight() - 8; 
    invalidateBitmap();;
}

pixel ClipView::requiredWidth() const
{
    return getRightPosition() - getLeftPosition();
}

pixel ClipView::requiredHeight() const
{
    if (mClip->isA<model::Transition>())
    {
        return Layout::sTransitionHeight;
    }
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

    // todo add if is transition and add three enums for begin, end, middle of transition


    ASSERT(dist_begin >= 0 && dist_end >= 0)(dist_begin)(dist_end);

    if (dist_begin < 10)
    {
        info.logicalclipposition = ClipBegin;
    }
    else if (dist_end < 10)
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

void ClipView::updateThumbnail(bool invalidate)
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(mClip);
    if (videoclip)
    {
        mClip->moveTo(0);
        model::VideoFramePtr videoFrame = videoclip->getNextVideo(getWidth() - 2 * Layout::sClipBorderSize, getHeight() - 2 * Layout::sClipBorderSize - Layout::sClipDescriptionBarHeight, false);
        mThumbnail = videoFrame->getBitmap();
        mClip->moveTo(0);
    }
    if (invalidate) invalidateBitmap();
}

void ClipView::draw(wxBitmap& bitmap) const
{
    draw(bitmap, !getDrag().isActive(), true);
}

void ClipView::draw(wxBitmap& bitmap, bool drawDraggedClips, bool drawNotDraggedClips) const
{
    // NOTE: DO NOT use getHeight here, since
    //       This method is also used for drawing clips that are
    //       dragged around, and thus can have a different size.
    //       (caused by hovering such a clip over another track
    //       typically causes it to be drawn with a different
    //       height).

    wxMemoryDC dc(bitmap);

    if (mClip->isA<model::EmptyClip>() || 
        (!drawDraggedClips && getDrag().contains(mClip)) ||
        (!drawNotDraggedClips && !getDrag().contains(mClip)))
    {
        // For empty clips, the bitmap is empty.
        // Selected clips that are being dragged should no longer be drawn
        // in the regular tracks as they have become part of 'getDrag()'s bitmap.
        dc.SetBrush(Layout::sBackgroundBrush);
        dc.SetPen(Layout::sBackgroundPen);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    }
    else if (mClip->isA<model::Transition>())
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

        // Thumbnail
        if (mThumbnail)
        {
            dc.DrawBitmap(*mThumbnail,wxPoint(Layout::sClipBorderSize, Layout::sClipDescriptionBarHeight));
        }

        // Text at top of clip
        dc.SetFont(*Layout::sClipDescriptionFont);
        dc.SetTextForeground(Layout::sClipDescriptionFGColour);
        dc.SetTextBackground(Layout::sClipDescriptionBGColour);
        dc.SetBrush(Layout::sClipDescriptionBrush);
        dc.SetPen(Layout::sClipDescriptionPen);
        //dc.SetLogicalFunction(wxEQUIV);
        dc.DrawRectangle(0,0,bitmap.GetWidth(), Layout::sClipDescriptionBarHeight);
        dc.DrawText(mClip->getDescription(), wxPoint(1,1));
    }

    if (mRect.GetHeight() != 0)
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawRectangle(mRect);
    }

    if (wxConfigBase::Get()->ReadBool(Config::sPathShowDebugInfoOnWidgets,false))
    {
        dc.SetTextForeground(Layout::sDebugColour);
        dc.SetFont(*Layout::sDebugFont);
        dc.DrawText(wxString::Format(wxT("%lld"), mClip->getLength()), wxPoint(5,15));
        wxString sPts; 
        sPts << '[' << mClip->getLeftPts() << ',' << mClip->getRightPts() << ')';
        dc.DrawText(sPts, wxPoint(5,25));

        pts progress = mClip->getGenerationProgress();
        pixel pos = getZoom().ptsToPixels(progress);
        dc.SetPen(Layout::sDebugPen);
        dc.DrawLine(wxPoint(pos,0), wxPoint(pos,bitmap.GetHeight()));
    }
}

void ClipView::drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const
{
    if (getDrag().contains(mClip))
    {
        wxBitmap b(getWidth(), height);
        draw(b, true, false);
        dc.DrawBitmap(b,position);
        dcMask.DrawRectangle(position,b.GetSize());
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void ClipView::onClipDragged( model::EventDragClip& event )
{
    invalidateBitmap();
    event.Skip();
}

void ClipView::onClipSelected( model::EventSelectClip& event )
{
    invalidateBitmap();
    event.Skip();
}

void ClipView::onGenerationProgress( model::DebugEventRenderProgress& event )
{
    invalidateBitmap();
    event.Skip();
}

}} // namespace
