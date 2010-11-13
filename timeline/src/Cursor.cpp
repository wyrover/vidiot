#include "Cursor.h"

#include "Constants.h"
#include "Intervals.h"
#include "Zoom.h"
#include "UtilLog.h"
#include "Timeline.h"
#include "GuiPlayer.h"
#include "VideoDisplayEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Cursor::Cursor()
:   wxEvtHandler()
,   mCursorPosition(0)
{
}

void Cursor::init()
{
    getPlayer()->Bind(EVENT_PLAYBACK_POSITION, &Cursor::onPlaybackPosition, this);
}


Cursor::~Cursor()
{
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
    VAR_DEBUG(mCursorPosition)(position);

    long oldPos = mCursorPosition;
    mCursorPosition = position;

    wxPoint scroll = getTimeline().getScrollOffset();

    // Refresh the old and new cursor position areas
    long cursorOnClientArea = mCursorPosition - scroll.x;
    long oldposOnClientArea = oldPos - scroll.x;
    getTimeline().RefreshRect(wxRect(std::min(cursorOnClientArea,oldposOnClientArea),0,std::abs(cursorOnClientArea-oldposOnClientArea)+1,getTimeline().getHeight()),false);

    getIntervals().update(mCursorPosition);
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
    dc.SetPen(Constants::sCursorPen);
    dc.DrawLine(wxPoint(mCursorPosition,0),wxPoint(mCursorPosition,getTimeline().getHeight()));
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
