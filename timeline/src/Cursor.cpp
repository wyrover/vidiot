// Copyright 2013 Eric Raijmakers.
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

#include "Cursor.h"

#include "Intervals.h"
#include "Layout.h"
#include "Player.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "VideoDisplayEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline {

const int Cursor::EDGE_OFFSET = 20;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Cursor::Cursor(Timeline* timeline)
    :   wxEvtHandler()
    ,   Part(timeline)
    ,   mCursorPosition(0)
{
    VAR_DEBUG(this);

    getPlayer()->Bind(EVENT_PLAYBACK_POSITION, &Cursor::onPlaybackPosition, this);
}

Cursor::~Cursor()
{
    VAR_DEBUG(this);

    getPlayer()->Unbind(EVENT_PLAYBACK_POSITION, &Cursor::onPlaybackPosition, this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

pts Cursor::getLogicalPosition() const
{
    return mCursorPosition;
}

void Cursor::setLogicalPosition(pts position)
{
    VAR_DEBUG(position);
    if (position >= 0 && position <= getZoom().pixelsToPts(getSequenceView().getSize().x)) // avoid out of bounds
    {
        moveTo(position);
        getPlayer()->moveTo(mCursorPosition);
    }
}

void Cursor::prevFrame()
{
    setLogicalPosition(getLogicalPosition() - 1);
    ensureCursorVisible();
}

void Cursor::nextFrame()
{
    setLogicalPosition(getLogicalPosition() + 1);
    ensureCursorVisible();
}

void Cursor::prevCut()
{
    std::set<pts> cuts = getSequence()->getCuts(); // std::set is stored in ordered fashion
    pts current = getLogicalPosition();
    BOOST_REVERSE_FOREACH( pts position, getSequence()->getCuts() )
    {
        if (position < current)
        {
            setLogicalPosition(position);
            ensureCursorVisible();
            return;
        }
    }
}

void Cursor::nextCut()
{
    std::set<pts> cuts = getSequence()->getCuts(); // std::set is stored in ordered fashion
    pts current = getLogicalPosition();
    BOOST_FOREACH( pts position, getSequence()->getCuts() )
    {
        if (position > current)
        {
            setLogicalPosition(position);
            ensureCursorVisible();
            return;
        }
    }
}

void Cursor::home()
{
    setLogicalPosition(0);
    ensureCursorVisible();
}

void Cursor::end()
{
    setLogicalPosition(getSequence()->getLength());
    ensureCursorVisible();
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Cursor::draw(wxDC& dc) const
{
    dc.SetPen(Layout::get().CursorPen);
    pixel pos = getZoom().ptsToPixels(mCursorPosition) - getTimeline().getShift();
    dc.DrawLine(wxPoint(pos,0),wxPoint(pos,getSequenceView().getSize().GetHeight()));
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void Cursor::onPlaybackPosition(PlaybackPositionEvent& event)
{
    moveTo(event.getValue());

    // Ensure that the scrolling is adjusted if playback moves the cursor beyond the visible region.
    // Note: specifically located here since a slightly different adjustment mechanism is used for user actions.
    wxPoint scroll = getScrolling().getOffset();
    wxSize size = getTimeline().GetClientSize();

    // Right side
    pts lastVisibleFrame = getZoom().pixelsToPts(scroll.x + size.x - EDGE_OFFSET);
    if (lastVisibleFrame < mCursorPosition)
    {
        getScrolling().align(mCursorPosition, size.x - EDGE_OFFSET);
    }

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Cursor::moveTo(pts position)
{
    if (position != mCursorPosition)
    {
        VAR_DEBUG(mCursorPosition)(position);

        long oldPixelPos = getZoom().ptsToPixels(mCursorPosition);
        long newPixelPos = getZoom().ptsToPixels(position);
        mCursorPosition = position;

        if (oldPixelPos != newPixelPos)
        {
            wxPoint scroll = getScrolling().getOffset();
            wxSize size = getTimeline().GetClientSize();

            // Refresh the old and new cursor position areas
            long cursorOnClientArea = newPixelPos - scroll.x;
            long oldposOnClientArea = oldPixelPos - scroll.x;
            getTimeline().RefreshRect(wxRect(cursorOnClientArea,0,1,getSequenceView().getSize().GetHeight()),false);
            getTimeline().RefreshRect(wxRect(oldposOnClientArea,0,1,getSequenceView().getSize().GetHeight()),true);
            getTimeline().Update(); // Use this for better feedback when dragging cursor..

            getIntervals().update(mCursorPosition);
        }
    }
}

void Cursor::ensureCursorVisible()
{
    // Ensure that the scrolling is adjusted if the cursor is moved outside the visible region by a user action
    // Note: the mechanism for adjusting the cursor during playback is specifically located elsewhere since
    //       a slightly different adjustment mechanism is used then.
    wxPoint scroll = getScrolling().getOffset();
    wxSize size = getTimeline().GetClientSize();

    // Left side
    pts firstVisibleFrame = getZoom().pixelsToPts(scroll.x);
    if (firstVisibleFrame > mCursorPosition)
    {
        getScrolling().align(mCursorPosition, EDGE_OFFSET);
    }
    // Right side
    pts lastVisibleFrame = getZoom().pixelsToPts(scroll.x + size.x);
    if (lastVisibleFrame < mCursorPosition)
    {
        getScrolling().align(mCursorPosition, size.x - EDGE_OFFSET);
    }

}

}} // namespace