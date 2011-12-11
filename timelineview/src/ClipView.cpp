#include "ClipView.h"

#include <wx/dcmemory.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>
#include "Config.h"
#include "Clip.h"
#include "ClipEvent.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "Layout.h"
#include "Options.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "SequenceView.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoFrame.h"
#include "ViewMap.h"
#include "Zoom.h"

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

pts ClipView::getLeftPts() const
{
    pts left = mClip->getLeftPts();
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mClip->getPrev());
    if (transition && transition->getRight() > 0)
    {
        ASSERT(!mClip->isA<model::Transition>());
        left -= transition->getRight();
    }
    return left;
}

pts ClipView::getRightPts() const
{
    pts right = mClip->getRightPts();
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mClip->getNext());
    if (transition && transition->getLeft() > 0)
    {
        ASSERT(!mClip->isA<model::Transition>());
        right += transition->getLeft();
    }
    return right;
}

pixel ClipView::getLeftPixel() const
{
    return getZoom().ptsToPixels(getLeftPts());
}

pixel ClipView::getRightPixel() const
{
    return getZoom().ptsToPixels(getRightPts());
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
    return getRightPixel() - getLeftPixel();
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
    pixel dist_begin = position.x - getLeftPixel();
    pixel dist_end = getRightPixel() - position.x;
    ASSERT_MORE_THAN_EQUALS_ZERO(dist_begin);
    ASSERT_MORE_THAN_EQUALS_ZERO(dist_end);

    model::TrackPtr track = mClip->getTrack();
    ASSERT_EQUALS(track,info.track);

    if (mClip->isA<model::Transition>())
    {
        pixel dist_top = position.y - info.trackPosition;
        ASSERT_MORE_THAN_EQUALS_ZERO(dist_top);

        if (dist_top <= Layout::sTransitionHeight)
        {
            info.logicalclipposition =
             (dist_begin < Layout::sCursorClipEditDistance)     ? TransitionBegin :
             (dist_end < Layout::sCursorClipEditDistance)       ? TransitionEnd :
             TransitionInterior; // Default
        }
        else // below transition
        {
            model::TransitionPtr transition = boost::static_pointer_cast<model::Transition>(mClip);
            pixel cut = getZoom().ptsToPixels(transition->getTouchPosition());
            pixel dist_cut = position.x - cut;

            if (dist_cut < 0)
            {
                info.logicalclipposition =
                    (dist_cut > -Layout::sCursorClipEditDistance) ? TransitionLeftClipEnd : TransitionLeftClipInterior;
            }
            else // (dist_cut >= 0)
            {
                info.logicalclipposition =
                    (dist_cut < Layout::sCursorClipEditDistance) ? TransitionRightClipBegin : TransitionRightClipInterior;
            }
        }
    }
    else// Regular clip
    {
        model::IClipPtr next = mClip->getNext();
        model::IClipPtr prev = mClip->getPrev();
        if ((dist_begin < Layout::sCursorClipEditDistance) && (!prev || !prev->isA<model::Transition>()))
        {
            info.logicalclipposition = ClipBegin;
        }
        else if ((dist_end < Layout::sCursorClipEditDistance) && (!next || !next->isA<model::Transition>()))
        {
            info.logicalclipposition = ClipEnd;
        }
        else
        {
            info.logicalclipposition = ClipInterior;
        }
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
        // Selected clips/transitions that are being dragged should no longer be drawn
        // in the regular tracks as they have become part of 'getDrag()'s bitmap.
        dc.SetBrush(Layout::sBackgroundBrush);
        dc.SetPen(Layout::sBackgroundPen);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    }
    else if (mClip->isA<model::Transition>())
    {
        if (mClip->getSelected())
        {
            dc.SetBrush(Layout::sTransitionBgSelected);
        }
        else
        {
            dc.SetBrush(Layout::sTransitionBgUnselected);
        }
        dc.DrawRectangle(0,0,bitmap.GetWidth(),Layout::sTransitionHeight);
        dc.SetPen(Layout::sTransitionPen);
        dc.SetBrush(Layout::sTransitionBrush);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),Layout::sTransitionHeight);
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

    if (Config::getShowDebugInfo())
    {
        if (!mClip->isA<model::Transition>())
        {
            dc.SetTextForeground(Layout::sDebugColour);
            dc.SetFont(*Layout::sDebugFont);
            dc.DrawText(wxString::Format(wxT("%lld"), mClip->getLength()), wxPoint(5,15));
            wxString sPts;
            sPts << '[' << mClip->getLeftPts() << ',' << mClip->getRightPts() << ')';
            dc.DrawText(sPts, wxPoint(5,25));
        }
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