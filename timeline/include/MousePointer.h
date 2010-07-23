#ifndef POINTERS_H
#define POINTERS_H

#include <wx/gdicmn.h>
#include <wx/cursor.h>
#include "UtilEnum.h"

namespace gui { namespace timeline {

class GuiTimeLine;

DECLAREENUM(MousePosition, \
            MouseBetweenClips, \
            MouseOnClipBegin, \
            MouseOnClipInterior, \
            MouseOnClipEnd, \
            MouseNotOnClip);

DECLAREENUM(MousePointerImage, \
            PointerNormal, \
            PointerMoveCut, \
            PointerTrimBegin, \
            PointerTrimShiftBegin, \
            PointerTrimEnd, \
            PointerTrimShiftEnd);

class MousePointer
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    MousePointer(GuiTimeLine& timeline);

    virtual ~MousePointer();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void set(MousePointerImage image);

    MousePosition getLogicalPosition(wxPoint virtualposition) const;

private:
    GuiTimeLine& mTimeline;

    wxCursor mCursorNormal;
    wxCursor mCursorMoveCut;
    wxCursor mCursorTrimBegin;
    wxCursor mCursorTrimShiftBegin;
    wxCursor mCursorTrimEnd;
    wxCursor mCursorTrimShiftEnd;
};

}} // namespace

#endif // POINTERS_H