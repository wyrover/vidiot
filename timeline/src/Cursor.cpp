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
    setPosition(getZoom().ptsToPixels(position));
}

void Cursor::moveCursorOnPlayback(pts position)
{
    setLogicalPosition(position);
}

void Cursor::moveCursorOnUser(pixel position)
{
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