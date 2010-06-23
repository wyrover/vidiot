#ifndef POINTERS_H
#define POINTERS_H

#include <wx/gdicmn.h>
#include <wx/cursor.h>

class GuiTimeLine;

class Pointers
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Pointers(GuiTimeLine& timeline);

    virtual ~Pointers();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool isOnBeginOfClip(wxPoint virtualposition);
    bool isOnEndOfClip(wxPoint virtualposition);
    bool isBetweenClips(wxPoint virtualposition);

private:
    GuiTimeLine& mTimeline;

    wxCursor mCursorMoveCut;
    wxCursor mCursorTrimBegin;
    wxCursor mCursorTrimEnd;
};

#endif // POINTERS_H