// Copyright 2013,2014 Eric Raijmakers.
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

#include "ClipView.h"

#include "Clip.h"
#include "ClipEvent.h"
#include "Config.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "Layout.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "SequenceView.h"
#include "ThumbnailView.h"
#include "Drag_Shift.h"
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

ClipView::ClipView(const model::IClipPtr& clip, View* parent)
    :   View(parent)
    ,   mClip(clip)
    ,   mBeginAddition(0)
    ,   mBitmap(boost::none)
{
    VAR_DEBUG(this)(mClip);
    ASSERT(mClip);

    getViewMap().registerView(mClip,this);

    if (mClip->isA<model::VideoClip>())
    {
        new ThumbnailView(clip,this);
    }
    mClip->Bind(model::EVENT_DRAG_CLIP,             &ClipView::onClipDragged,           this);
    mClip->Bind(model::EVENT_SELECT_CLIP,           &ClipView::onClipSelected,          this);
    mClip->Bind(model::DEBUG_EVENT_RENDER_PROGRESS, &ClipView::onGenerationProgress,    this);

    // IMPORTANT: No drawing/lengthy code here. Due to the nature of adding removing clips as
    //            part of edit operations, that will severely impact performance.
}

ClipView::~ClipView()
{
    VAR_DEBUG(this);

    mClip->Unbind(model::EVENT_DRAG_CLIP,             &ClipView::onClipDragged,         this);
    mClip->Unbind(model::EVENT_SELECT_CLIP,           &ClipView::onClipSelected,        this);
    mClip->Unbind(model::DEBUG_EVENT_RENDER_PROGRESS, &ClipView::onGenerationProgress,  this);

    if (mClip->isA<model::VideoClip>())
    {
        delete getViewMap().getThumbnail(mClip);
    }
    getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel ClipView::getX() const
{
    return getParent().getX() + getLeftPixel() + getShift();
}

pixel ClipView::getY() const
{
    return getParent().getY();
}

pixel ClipView::getW() const
{
    return getRightPixel() - getLeftPixel();
}

pixel ClipView::getH() const
{
    return (mClip->isA<model::Transition>()) ? Layout::TransitionHeight : getParent().getH();
}

void ClipView::invalidateRect()
{
    mBitmap.reset();
    if (mClip->isA<model::VideoClip>())
    {
        getViewMap().getThumbnail(mClip)->invalidateRect();
    }
}

void ClipView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (mClip->getDragged())
    {
        getTimeline().clearRect(dc,region,offset,getRect());
    }
    else
    {
        wxSize size(getSize());
        if (!mBitmap || mBitmap->GetSize() != size)
        {
            mBitmap.reset(wxBitmap(size));
            draw(*mBitmap, !getDrag().isActive(), true);
        }
        getTimeline().copyRect(dc, region, offset, *mBitmap, getRect());
        if (mClip->isA<model::VideoClip>())
        {
            getViewMap().getThumbnail(mClip)->draw(dc,region,offset);
        }
    }
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
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mClip->getInTransition());
    if (transition)
    {
        ASSERT(!mClip->isA<model::Transition>());
        boost::optional<pts> right = transition->getRight();
        if (right)
        {
            left -= *right;
        }
    }
    return left;
}

pts ClipView::getRightPts() const
{
    pts right = mClip->getRightPts();
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mClip->getOutTransition());
    if (transition)
    {
        ASSERT(!mClip->isA<model::Transition>());
        boost::optional<pts> left = transition->getLeft();
        if (left)
        {
            right += *left;
        }
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

pixel ClipView::getShift() const
{
    Shift shift = getDrag().getShift();
    if (shift && getLeftPts() >= shift->getPtsPosition())
    {
        return getZoom().ptsToPixels(shift->getPtsLength());
    }
    return 0;
}

void ClipView::getPositionInfo(const wxPoint& position, PointerPositionInfo& info) const
{
    ASSERT(info.track); // If the track is not filled in, then how can this clipview be reached?

    // This is handled on a per-pixel and not per-pts basis. That ensures
    // that this still works for clips which are very small when zoomed out.
    // (then the cursor won't flip too much).
    pixel dist_begin = position.x - getLeftPixel();
    pixel dist_end = getRightPixel() - position.x;
    ASSERT_MORE_THAN_EQUALS_ZERO(dist_begin)(mClip);
    ASSERT_MORE_THAN_EQUALS_ZERO(dist_end)(mClip);

    model::TrackPtr track = mClip->getTrack();
    ASSERT_EQUALS(track,info.track);

    if (mClip->isA<model::Transition>())
    {
        pixel dist_top = position.y - info.trackPosition;
        ASSERT_MORE_THAN_EQUALS_ZERO(dist_top);

        if (dist_top <= Layout::TransitionHeight)
        {
            info.logicalclipposition =
                (dist_begin < Layout::CursorClipEditDistance)     ? TransitionBegin :
                (dist_end < Layout::CursorClipEditDistance)       ? TransitionEnd :
                TransitionInterior; // Default
        }
        else // below transition
        {
            model::TransitionPtr transition = boost::static_pointer_cast<model::Transition>(mClip);
            pixel cut = getZoom().ptsToPixels(transition->getTouchPosition());
            pixel dist_cut = position.x - cut;

            if (dist_cut < 0)
            {
                ASSERT(transition->getLeft());
                ASSERT_MORE_THAN_ZERO(*(transition->getLeft()));
                if (dist_cut > -Layout::CursorClipEditDistance)
                {
                    info.logicalclipposition = TransitionLeftClipEnd;
                }
                else
                {
                    // First determine if the pointer is 'near' the begin of the 'in' clip of the transition.
                    // If that is the case, use that clip and not the transition.
                    model::IClipPtr inClip = transition->getPrev();
                    pixel inpoint = getZoom().ptsToPixels(inClip->getLeftPts());
                    pixel dist_left = position.x - inpoint;
                    ASSERT_MORE_THAN_EQUALS_ZERO(dist_left)(inpoint)(position);

                    if (dist_left < Layout::CursorClipEditDistance)
                    {
                        // Logically, the pointer is hovering 'over' the clip left of the transition
                        info.clip = inClip;
                        info.logicalclipposition = ClipBegin;
                    }
                    else
                    {
                        info.logicalclipposition = TransitionLeftClipInterior;
                    }
                }
            }
            else if (dist_cut > 0)
            {
                ASSERT(transition->getRight());
                ASSERT_MORE_THAN_ZERO(*(transition->getRight()));
                if (dist_cut < Layout::CursorClipEditDistance)
                {
                    info.logicalclipposition = TransitionRightClipBegin;
                }
                else
                {
                    // First determine if the pointer is 'near' the end of the 'out' clip of the transition.
                    // If that is the case, use that clip and not the transition.
                    model::IClipPtr outClip = transition->getNext();
                    pixel outpoint = getZoom().ptsToPixels(outClip->getRightPts());
                    pixel dist_right = outpoint - position.x;
                    ASSERT_MORE_THAN_EQUALS_ZERO(dist_right)(outpoint)(position);

                    if (dist_right < Layout::CursorClipEditDistance)
                    {
                        // Logically, the pointer is hovering 'over' the clip right of the transition
                        info.clip = outClip;
                        info.logicalclipposition = ClipEnd;
                    }
                    else
                    {
                        info.logicalclipposition = TransitionRightClipInterior;
                    }
                }
            }
            else // dist_cut == 0
            {
                if (transition->getRight() &&
                    *(transition->getRight()) > 0)
                {
                    info.logicalclipposition = TransitionRightClipBegin;
                }
                else
                {
                    ASSERT(transition->getLeft());
                    ASSERT_MORE_THAN_ZERO(*(transition->getLeft()));
                    info.logicalclipposition = TransitionLeftClipEnd;
                }
            }

        }
    }
    else// Regular clip
    {
        model::IClipPtr next = mClip->getNext();
        model::IClipPtr prev = mClip->getPrev();
        if ((dist_begin < Layout::CursorClipEditDistance) && (!prev || !prev->isA<model::Transition>()))
        {
            info.logicalclipposition = ClipBegin;
        }
        else if ((dist_end < Layout::CursorClipEditDistance) && (!next || !next->isA<model::Transition>()))
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

void ClipView::draw(wxBitmap& bitmap, bool drawDraggedClips, bool drawNotDraggedClips) const
{
    // NOTE: DO NOT use getHeight here, since
    //       This method is also used for drawing clips that are
    //       dragged around, and thus can have a different size.
    //       (caused by hovering such a clip over another track
    //       typically causes it to be drawn with a different
    //       height).

    wxMemoryDC dc(bitmap);

    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();
    //int r = getRightPixel();
    //int l = getLeftPixel();

    if (mClip->isA<model::EmptyClip>() ||
        (!drawDraggedClips && getDrag().contains(mClip)) ||
        (!drawNotDraggedClips && !getDrag().contains(mClip)))
    {
        // For empty clips, the bitmap is empty.
        // Selected clips/transitions that are being dragged should no longer be drawn
        // in the regular tracks as they have become part of 'getDrag()'s bitmap.
        dc.SetBrush(Layout::get().BackgroundBrush);
        dc.SetPen(Layout::get().BackgroundPen);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    }
    else if (mClip->isA<model::Transition>())
    {
        if (mClip->getSelected())
        {
            dc.SetBrush(Layout::get().TransitionBgSelected);
        }
        else
        {
            dc.SetBrush(Layout::get().TransitionBgUnselected);
        }
        dc.DrawRectangle(0,0,bitmap.GetWidth(),Layout::TransitionHeight);
        dc.SetPen(Layout::get().TransitionPen);
        dc.SetBrush(Layout::get().TransitionBrush);
        dc.DrawRectangle(0,0,bitmap.GetWidth(),Layout::TransitionHeight);
    }
    else
    {
        if (mClip->getSelected())
        {
            dc.SetBrush(Layout::get().SelectedClipBrush);
            dc.SetPen(Layout::get().SelectedClipPen);
        }
        else
        {
            dc.SetBrush(Layout::get().ClipBrush);
            dc.SetPen(Layout::get().ClipPen);
        }
        dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());

        // Text at top of clip
        dc.SetFont(Layout::get().ClipDescriptionFont);
        dc.SetTextForeground(Layout::get().ClipDescriptionFGColour);
        dc.SetTextBackground(Layout::get().ClipDescriptionBGColour);
        dc.SetBrush(Layout::get().ClipDescriptionBrush);
        dc.SetPen(Layout::get().ClipDescriptionPen);
        //dc.SetLogicalFunction(wxEQUIV);
        dc.DrawRectangle(0,0,bitmap.GetWidth(), Layout::ClipDescriptionBarHeight);
        dc.DrawText(mClip->getDescription(), wxPoint(1,1));
    }

    if (Config::getShowDebugInfo())
    {
        if (!mClip->isA<model::Transition>())
        {
            dc.SetTextForeground(Layout::get().DebugColour);
            dc.SetFont(Layout::get().DebugFont);
            dc.DrawText(wxString::Format("%" PRId64, mClip->getLength()), wxPoint(5,15));
            wxString sPts;
            sPts << '[' << mClip->getLeftPts() << ',' << mClip->getRightPts() << ')';
            dc.DrawText(sPts, wxPoint(5,25));
        }
        pts progress = mClip->getGenerationProgress();
        pixel pos = getZoom().ptsToPixels(progress);
        dc.SetPen(Layout::get().DebugPen);
        dc.DrawLine(wxPoint(pos,0), wxPoint(pos,bitmap.GetHeight()));
    }
}

void ClipView::drawForDragging(const wxPoint& position, int height, wxDC& dc, wxDC& dcMask) const
{
    if (getDrag().contains(mClip))
    {
        wxBitmap b(getW(),height);
        draw(b, true, false);
// todo move include atomic to precompiled
        // Don't use DrawBitmap since this gives wrong output when using wxGTK.
        wxMemoryDC dcBmp(b);
        dc.Blit(position, b.GetSize(), &dcBmp, wxPoint(0,0));
        if (mClip->isA<model::VideoClip>())
        {
            getViewMap().getThumbnail(mClip)->drawForDragging(position, height, dc);
        }
        dcMask.DrawRectangle(position,b.GetSize());
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void ClipView::onClipDragged(model::EventDragClip& event )
{
    repaint();
    event.Skip();
}

void ClipView::onClipSelected(model::EventSelectClip& event )
{
    mBitmap.reset();
    repaint();
    event.Skip();
}

void ClipView::onGenerationProgress( model::DebugEventRenderProgress& event )
{
    if (Config::getShowDebugInfo())
    {
        mBitmap.reset();
        repaint();
    }
    event.Skip();
}
}} // namespace
