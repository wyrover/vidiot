#include "Cursor.h"

#include "Layout.h"
#include "Intervals.h"
#include "Zoom.h"
#include "UtilLog.h"
#include "Timeline.h"
#include "GuiPlayer.h"

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

long Cursor::getPosition() const
{ 
    return mCursorPosition; 
}

void Cursor::setPosition(long position)
{
    if (position != mCursorPosition)
    {
        VAR_DEBUG(mCursorPosition)(position);

        long oldPos = mCursorPosition;
        mCursorPosition = position;

        wxPoint scroll = getTimeline().getScrollOffset();

        // Refresh the old and new cursor position areas
        getTimeline().invalidateBitmap();
        long cursorOnClientArea = mCursorPosition - scroll.x;
        long oldposOnClientArea = oldPos - scroll.x;
        getTimeline().RefreshRect(wxRect(std::min(cursorOnClientArea,oldposOnClientArea),0,std::abs(cursorOnClientArea-oldposOnClientArea)+1,getTimeline().requiredHeight()),false);
        getTimeline().Update(); // Use this for better feedback when dragging cursor..

        getIntervals().update(mCursorPosition);
    }
}

void Cursor::moveCursorOnPlayback(long pts)
{
    setPosition(getZoom().ptsToPixels(pts));
}

void Cursor::moveCursorOnUser(long position)
{
    setPosition(position);
    getPlayer()->moveTo(getZoom().pixelsToPts(position));
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Cursor::draw(wxDC& dc) const
{
    dc.SetPen(Layout::sCursorPen);
    dc.DrawLine(wxPoint(mCursorPosition,0),wxPoint(mCursorPosition,getTimeline().requiredHeight()));
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
