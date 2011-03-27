#ifndef POINTERS_H
#define POINTERS_H

#include <wx/gdicmn.h>
#include <wx/cursor.h>
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

class MousePointer
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    MousePointer(Timeline* timeline);
    virtual ~MousePointer();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void set(MousePointerImage image);

    /**
    * Return info on the given position.
    **/
    PointerPositionInfo getInfo(wxPoint pointerposition);

    
    void setLeftDownPosition(wxPoint position);  ///< \param position last (most recent) position at which a 'left down' event was received.
    wxPoint getLeftDownPosition() const;    ///< \return last (most recent) position at which a 'left down' event was received.
    void setRightDownPosition(wxPoint position); ///< \param position last (most recent) position at which a 'right down' event was received.
    wxPoint getRightDownPosition() const;   ///< \return last (most recent) position at which a 'right down' event was received.

private:

    wxCursor mCursorNormal;
    wxCursor mCursorMoveCut;
    wxCursor mCursorTrimBegin;
    wxCursor mCursorTrimShiftBegin;
    wxCursor mCursorTrimEnd;
    wxCursor mCursorTrimShiftEnd;
    wxCursor mCursorTrackResize;

    wxPoint mLeft;
    wxPoint mRight;
};

}} // namespace

#endif // POINTERS_H