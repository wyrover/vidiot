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

#include "ClipView.h"

#include "Clip.h"
#include "ClipEvent.h"
#include "Config.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "SequenceView.h"
#include "ThumbnailView.h"
#include "Drag_Shift.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilWindow.h"
#include "VideoClip.h"
#include "VideoFrame.h"
#include "ViewMap.h"
#include "Zoom.h"

namespace gui {
namespace timeline {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

const pixel ClipView::BorderSize{ 2 };
const pixel ClipView::CursorClipEditDistance{ 6 };

ClipView::ClipView(const model::IClipPtr& clip, View* parent)
    : View(parent)
    , mClip(clip)
    , mBeginAddition(0)
    , mBitmap(boost::none)
{
    VAR_DEBUG(this)(mClip);
    ASSERT(mClip);

    getViewMap().registerView(mClip, this);

    if (mClip->isA<model::VideoClip>())
    {
        (new ThumbnailView(clip, this));
    }
    else if (mClip->isA<model::AudioClip>())
    {
        (new AudioPeakView(clip, this));
    }
    mClip->Bind(model::EVENT_DRAG_CLIP, &ClipView::onClipDragged, this);
    mClip->Bind(model::EVENT_SELECT_CLIP, &ClipView::onClipSelected, this);
    mClip->Bind(model::EVENT_CHANGE_CLIP_KEYFRAMES, &ClipView::onKeyFramesChanged, this);

    // IMPORTANT: No drawing/lengthy code here. Due to the nature of adding removing clips as
    //            part of edit operations, that will severely impact performance.
}

ClipView::~ClipView()
{
    VAR_DEBUG(this);

    mClip->Unbind(model::EVENT_DRAG_CLIP, &ClipView::onClipDragged, this);
    mClip->Unbind(model::EVENT_SELECT_CLIP, &ClipView::onClipSelected, this);
    mClip->Unbind(model::EVENT_CHANGE_CLIP_KEYFRAMES, &ClipView::onKeyFramesChanged, this);

    if (mClip->isA<model::VideoClip>())
    {
        delete getViewMap().getClipPreview(mClip);
    }
    else if (mClip->isA<model::AudioClip>())
    {
        delete getViewMap().getClipPreview(mClip);
    }
    getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
// DRAWING PARAMETERS REUSED
//////////////////////////////////////////////////////////////////////////

int determineFontHeight(const wxFont& font)
{
    wxBitmap tmp(100, 100); // tmp, so size not that important.
    wxMemoryDC dc(tmp);
    dc.SetFont(font);
    return dc.GetCharHeight();
}

int determineDescriptionHeight()
{
    return determineFontHeight(wxFont{ wxSize(0, 11), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL }) + 2;
}

// static 
int ClipView::getBorderSize()
{
    return 2;
}

// static 
int ClipView::getDescriptionHeight()
{
    static int result{ determineDescriptionHeight() };
    return result;
}

// static 
int ClipView::getTransitionHeight()
{
    static int result{ getDescriptionHeight() + 4 };
    return result;
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
    return (mClip->isA<model::Transition>()) ? ClipView::getTransitionHeight() : getParent().getH();
}

void ClipView::invalidateRect()
{
    mBitmap.reset();
    if (mClip->isA<model::VideoClip>() || mClip->isA<model::AudioClip>())
    {
        getViewMap().getClipPreview(mClip)->invalidateRect();
    }
}

void ClipView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (mClip->getDragged())
    {
        getTimeline().clearRect(dc, region, offset, getRect());
    }
    else
    {
        wxSize size(getSize());
        if (size.x > 0 && size.y > 0)
        {
            // During some operations the resulting view may be of '0' size.
            if (!mBitmap || mBitmap->GetSize() != size)
            {
                mBitmap.reset(wxBitmap(size));
                draw(*mBitmap, !getDrag().isActive(), true);
            }
            getTimeline().copyRect(dc, region, offset, *mBitmap, getRect());
            if (!Config::getShowDebugInfo())
            {
                if (mClip->isA<model::VideoClip>() || mClip->isA<model::AudioClip>())
                {
                    getViewMap().getClipPreview(mClip)->draw(dc, region, offset);

                    if (auto interval{ boost::dynamic_pointer_cast<model::ClipInterval>(mClip) })
                    {
                        std::map<pts, model::KeyFramePtr> keyframes{ interval->getKeyFramesOfPerceivedClip() };
                        wxBitmap& kfi{ getKeyFrameIndicator() };
                        for (auto pts_frame : keyframes)
                        {
                            wxRect r{ getRect() };
                            r.x += getZoom().ptsToPixels(pts_frame.first) - kfi.GetWidth() / 2;
                            r.x = std::max(getX(), r.x);
                            r.y += getH() - kfi.GetHeight() - ClipView::BorderSize;
                            getTimeline().copyRect(dc, region, offset, kfi, r, true);
                        }
                    }
                }
            }
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
        ASSERT(!mClip->isA<model::Transition>())(mClip);
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
        ASSERT(!mClip->isA<model::Transition>())(mClip);
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
    ASSERT_EQUALS(track, info.track);

    if (mClip->isA<model::Transition>())
    {
        pixel dist_top = position.y - info.trackPosition;
        ASSERT_MORE_THAN_EQUALS_ZERO(dist_top);

        if (dist_top <= ClipView::getTransitionHeight())
        {
            info.logicalclipposition =
                (dist_begin < ClipView::CursorClipEditDistance) ? TransitionBegin :
                (dist_end < ClipView::CursorClipEditDistance) ? TransitionEnd :
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
                if (dist_cut > -ClipView::CursorClipEditDistance)
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

                    if (dist_left < ClipView::CursorClipEditDistance)
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
                if (dist_cut < ClipView::CursorClipEditDistance)
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

                    if (dist_right < ClipView::CursorClipEditDistance)
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
        model::TransitionPtr prevTransition = boost::dynamic_pointer_cast<model::Transition>(prev);
        model::TransitionPtr nextTransition = boost::dynamic_pointer_cast<model::Transition>(next);

        if ((dist_begin < ClipView::CursorClipEditDistance) && (!prev || !prevTransition))
        {
            info.logicalclipposition = ClipBegin;
        }
        else if ((dist_begin < ClipView::CursorClipEditDistance) && prevTransition && prevTransition->getRight())
        {
            info.logicalclipposition = TransitionRightClipBegin;
            info.clip = prevTransition;
        }
        else if ((dist_end < ClipView::CursorClipEditDistance) && (!next || !nextTransition))
        {
            info.logicalclipposition = ClipEnd;
        }
        else if ((dist_end < ClipView::CursorClipEditDistance) && nextTransition && nextTransition->getLeft())
        {
            info.logicalclipposition = TransitionLeftClipEnd;
            info.clip = nextTransition;
        }
        else
        {
            info.logicalclipposition = ClipInterior;
        }
    }
}

void ClipView::getKeyframePositionInfo(const wxPoint& position, PointerPositionInfo& info) const
{
    ASSERT_EQUALS(info.getLogicalClip(), mClip)(info)(mClip);

    std::vector<wxRect> keyFrameRects;
    if (auto interval{ boost::dynamic_pointer_cast<model::ClipInterval>(mClip) })
    {
        std::map<pts, model::KeyFramePtr> keyframes{ interval->getKeyFramesOfPerceivedClip() };
        wxBitmap& kfi{ getKeyFrameIndicator() };
        for (auto pts_frame : keyframes)
        {
            wxRect r{ getRect() };
            r.x += getZoom().ptsToPixels(pts_frame.first) - kfi.GetWidth() / 2;
            r.x = std::max(getX(), r.x);
            r.y += getH() - kfi.GetHeight() - ClipView::BorderSize;
            r.SetSize(kfi.GetSize());
            keyFrameRects.push_back(r);
        }
    }

    size_t keyFrameIndex{ 0 };
    info.keyframe.reset();
    for (auto r : keyFrameRects)
    {
        if (r.Contains(position))
        {
            info.keyframe.reset(keyFrameIndex);
            break;
        }
        ++keyFrameIndex;
    }
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void ClipView::drawForDragging(const wxPoint& position, int height, wxDC& dc, wxDC& dcMask) const
{
    if (getDrag().contains(mClip) &&
        getW() > 0)
    {
        if (mClip->isA<model::Transition>())
        {
            // Only the top halve of the track height is filled for transitions.
            // Do not use a 'too big' bitmap here, since it is blit'ed in its entirety.
            // Using a 'track height' bitmap causes the area under the transition to
            // become black during dragging.
            height = std::min(height, ClipView::getTransitionHeight());
        }
        wxBitmap b(getW(), height);
        draw(b, true, false);
        // Don't use DrawBitmap since this gives wrong output when using wxGTK.
        wxMemoryDC dcBmp(b);
        dc.Blit(position, b.GetSize(), &dcBmp, wxPoint(0, 0));
        if (mClip->isA<model::VideoClip>() ||
            mClip->isA<model::AudioClip>())
        {
            getViewMap().getClipPreview(mClip)->drawForDragging(position, height, dc);
        }
        dcMask.DrawRectangle(position, b.GetSize());
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void ClipView::onClipDragged(model::EventDragClip& event)
{
    repaint();
    event.Skip();
}

void ClipView::onClipSelected(model::EventSelectClip& event)
{
    mBitmap.reset();
    repaint();
    event.Skip();
}

void ClipView::onKeyFramesChanged(model::EventChangeClipKeyFrames& event)
{
    mBitmap.reset();
    repaint();
    event.Skip();
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

    if (mClip->isA<model::EmptyClip>() ||
        (!drawDraggedClips && getDrag().contains(mClip)) ||
        (!drawNotDraggedClips && !getDrag().contains(mClip)))
    {
        // For empty clips, the bitmap is empty.
        // Selected clips/transitions that are being dragged should no longer be drawn
        // in the regular tracks as they have become part of 'getDrag()'s bitmap.
        dc.SetBrush(wxBrush{ wxColour{ 212, 208, 200 } });
        dc.SetPen(wxPen{ wxColour{ 212, 208, 200 } });
        dc.DrawRectangle(0, 0, bitmap.GetWidth(), bitmap.GetHeight());
    }
    else if (mClip->isA<model::Transition>())
    {
        dc.SetBrush(wxBrush{ mClip->getSelected() ? wxColour{ 80, 80, 80 } : wxColour{ 123, 123, 123 } });
        dc.DrawRectangle(0, 0, bitmap.GetWidth(), ClipView::getTransitionHeight());
        wxColour linesColour{ 224, 0, 224 };
        dc.SetPen(wxPen{ linesColour, 1 });
        dc.SetBrush(wxBrush{ linesColour, wxBRUSHSTYLE_FDIAGONAL_HATCH });
        dc.DrawRectangle(0, 0, bitmap.GetWidth(), ClipView::getTransitionHeight());
    }
    else
    {
        wxColour borderColour{ 32, 32, 32 };
        dc.SetPen(wxPen{ borderColour, ClipView::BorderSize });
        dc.SetBrush(wxBrush{ mClip->getSelected() ? wxColour{ 80, 80, 80 } : wxColour{ 160, 160, 160 } });
        dc.DrawRectangle(0, 0, bitmap.GetWidth(), bitmap.GetHeight());

        // Text at top of clip
        wxFont descriptionFont{ wxSize(0, 11), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL };
        dc.SetFont(descriptionFont);
        dc.SetTextForeground(wxColour{ 255, 255, 255 });
        dc.SetTextBackground(borderColour);
        dc.SetBrush(wxBrush{ borderColour });
        dc.SetPen(wxPen{ borderColour, 1 });
        //dc.SetLogicalFunction(wxEQUIV);
        dc.DrawRectangle(0, 0, bitmap.GetWidth(), ClipView::getDescriptionHeight());
        dc.DrawText(mClip->getDescription(), wxPoint(1, 1));
    }

    if (Config::getShowDebugInfo())
    {
        if (!mClip->isA<model::Transition>())
        {
            dc.SetTextForeground(wxColour{ 0, 0, 0 });
            dc.SetFont(wxFont(wxSize(0, 11), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(wxString::Format("%" PRId64, mClip->getLength()), wxPoint(5, 16));
            if (mClip->isA<model::ClipInterval>())
            {
                dc.DrawText(wxString::Format("%" PRId64, boost::dynamic_pointer_cast<model::ClipInterval>(mClip)->getOffset()), wxPoint(45, 16));
            }
            wxString sPts;
            sPts << '[' << mClip->getLeftPts() << ',' << mClip->getRightPts() << ')';
            dc.DrawText(sPts, wxPoint(5, 30));
        }
    }
}

// static
wxBitmap& ClipView::getKeyFrameIndicator()
{
    static wxBitmap result;
    if (!result.IsOk()) 
    {
        result.Create(8, 8);
        wxBitmap mask{ result.GetSize(), 1 };
        wxMemoryDC dc{ result };
        dc.SetPen(wxPen{ wxColour{ 75,67,0 }, 1 });
        dc.SetBrush(wxBrush{ wxColour{ 255,255,0 }, wxBRUSHSTYLE_SOLID });
        dc.DrawRoundedRectangle(0, 0, result.GetWidth(), result.GetHeight(), 2);
        dc.SelectObject(wxNullBitmap);
        wxMemoryDC dcMask{ mask };
        dcMask.SetPen(wxPen{ wxColour{ 255,255,255 }, 1 });
        dcMask.SetBrush(wxBrush{ wxColour{ 255,255,255 }, wxBRUSHSTYLE_SOLID });
        dcMask.DrawRoundedRectangle(0, 0, result.GetWidth(), result.GetHeight(), 2);
        dcMask.SelectObject(wxNullBitmap);
        result.SetMask(new wxMask{ mask });
    }
    return result;
}

}} // namespace
