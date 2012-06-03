#ifndef CURSOR_H
#define CURSOR_H

#include <wx/event.h>
#include "Part.h"
#include "UtilInt.h"

namespace gui { class PlaybackPositionEvent; }

namespace gui { namespace timeline {

class Cursor
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Cursor(Timeline* timeline);
    virtual ~Cursor();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    pixel getPosition() const;
    void setPosition(pixel position);

    pts getLogicalPosition() const;
    void setLogicalPosition(pts position);

    void moveCursorOnUser(pixel position);
    void moveCursorOnPlayback(pts position);

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc) const;

private:

    pixel mCursorPosition;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPlaybackPosition(PlaybackPositionEvent& event);
};

}} // namespace

#endif // CURSOR_H