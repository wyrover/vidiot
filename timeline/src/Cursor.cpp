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
#include "SequenceView.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "VideoDisplayEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline {

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

pixel Cursor::getPosition() const
{
    return mCursorPosition;
}

void Cursor::setPosition(pixel position)
{
    if (position != mCursorPosition)
    {
        VAR_DEBUG(mCursorPosition)(position);

        long oldPos = mCursorPosition;
        mCursorPosition = position;

        wxPoint scroll = getScrolling().getOffset();

        // Refresh the old and new cursor position areas
        long cursorOnClientArea = mCursorPosition - scroll.x;
        long oldposOnClientArea = oldPos - scroll.x;
        getTimeline().RefreshRect(wxRect(cursorOnClientArea,0,1,getSequenceView().getSize().GetHeight()),false);
        getTimeline().RefreshRect(wxRect(oldposOnClientArea,0,1,getSequenceView().getSize().GetHeight()),true);
        getTimeline().Update(); // Use this for better feedback when dragging cursor..

        getIntervals().update(mCursorPosition);
    }
}

pts Cursor::getLogicalPosition() const
{
    return getZoom().pixelsToPts(mCursorPosition);
}

void Cursor::setLogicalPosition(pts position)
{
    VAR_DEBUG(position);
    setPosition(getZoom().ptsToPixels(position));
}

void Cursor::moveCursorOnPlayback(pts position)
{
    VAR_DEBUG(position);
    setLogicalPosition(position);
}

void Cursor::moveCursorOnUser(pixel position)
{
    VAR_DEBUG(position);
    setPosition(position);
    getPlayer()->moveTo(getZoom().pixelsToPts(position));
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Cursor::draw(wxDC& dc) const
{
    dc.SetPen(Layout::get().CursorPen);
    dc.DrawLine(wxPoint(mCursorPosition,0),wxPoint(mCursorPosition,getSequenceView().getSize().GetHeight()));
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void Cursor::onPlaybackPosition(PlaybackPositionEvent& event)
{
    moveCursorOnPlayback(event.getValue());
    event.Skip();
}

}} // namespace