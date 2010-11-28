#ifndef POINTERS_H
#define POINTERS_H

#include <wx/gdicmn.h>
#include <wx/cursor.h>
#include "UtilEnum.h"
#include "Part.h"

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    class Clip;
    typedef boost::shared_ptr<Clip> ClipPtr;
}

namespace gui { namespace timeline {

DECLAREENUM(MousePointerImage, \
            PointerNormal, \
            PointerMoveCut, \
            PointerTrimBegin, \
            PointerTrimShiftBegin, \
            PointerTrimEnd, \
            PointerTrimShiftEnd, \
            PointerTrackResize);

DECLAREENUM(MouseOnClipPosition, \
            ClipBetween, \
            ClipBegin, \
            ClipInterior, \
            ClipEnd);

/** @todo split into track info and 'withintrack' info */
struct PointerPositionInfo
{
    //////////////////////////////////////////////////////////////////////////
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    /** Current track under the mouse pointer. 0 if none. */
    model::TrackPtr track;

    /** Y position of current track. 0 if no current track. */
    int trackPosition;

    /** True if pointer is ON the track divider for this track. */
    bool onTrackDivider;

    //////////////////////////////////////////////////////////////////////////
    // CLIP
    //////////////////////////////////////////////////////////////////////////

    /** Current clip under the mouse pointer. 0 if none. */
    model::ClipPtr clip;

    MouseOnClipPosition logicalclipposition;

};

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

private:

    wxCursor mCursorNormal;
    wxCursor mCursorMoveCut;
    wxCursor mCursorTrimBegin;
    wxCursor mCursorTrimShiftBegin;
    wxCursor mCursorTrimEnd;
    wxCursor mCursorTrimShiftEnd;
    wxCursor mCursorTrackResize;
};

}} // namespace

#endif // POINTERS_H