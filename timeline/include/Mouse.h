#ifndef MOUSE_POINTER_H
#define MOUSE_POINTER_H

#include "UtilEnum.h"
#include "Part.h"
#include "UtilInt.h"

namespace gui { namespace timeline {
    struct PointerPositionInfo;

DECLAREENUM(MousePointerImage, \
            PointerNormal, \
            PointerMoveCut, \
            PointerTrimBegin, \
            PointerTrimShiftBegin, \
            PointerTrimEnd, \
            PointerTrimShiftEnd, \
            PointerTrackResize);

class Mouse
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Mouse(Timeline* timeline);
    virtual ~Mouse();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void update(const wxMouseState& state);
    void leftDown();
    void rightDown();

    void dragMove(wxPoint position);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void set(MousePointerImage image);

    PointerPositionInfo getInfo(wxPoint pointerposition); /// \return info on the given position.
    wxPoint getPhysicalPosition() const;     ///< \return current physical (unscrolled) positionm of mouse pointer
    wxPoint getVirtualPosition() const;     ///< \return current virtual (includes scrolling offset) position of mouse pointer
    wxPoint getLeftDownPosition() const;    ///< \return last (most recent) position at which a 'left down' event was received.
    wxPoint getRightDownPosition() const;   ///< \return last (most recent) position at which a 'right down' event was received.

private:

    wxCursor mCursorNormal;
    wxCursor mCursorMoveCut;
    wxCursor mCursorTrimBegin;
    wxCursor mCursorTrimShiftBegin;
    wxCursor mCursorTrimEnd;
    wxCursor mCursorTrimShiftEnd;
    wxCursor mCursorTrackResize;

    wxPoint mPhysicalPosition;
    wxPoint mVirtualPosition;
    wxPoint mLeft; ///< Holds the most recent position at which the left button was pressed.
    wxPoint mRight; ///< Holds the most recent position at which the left button was pressed.
};

}} // namespace

#endif // MOUSE_POINTER_H