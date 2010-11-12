#include "MousePointer.h"

#include <wx/image.h>
#include <wx/bitmap.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"
#include "cursor_move_cut.xpm"
#include "cursor_trim_begin.xpm"
#include "cursor_trim_end.xpm"
#include "cursor_normal.xpm"
#include "Track.h"
#include "Sequence.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MousePointerImage);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MousePointer::MousePointer()
{
    wxImage image;

    image = wxBitmap(cursor_normal_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorNormal = wxCursor(image);

    image = wxBitmap(cursor_move_cut_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorMoveCut = wxCursor(image);

    image = wxBitmap(cursor_trim_begin_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorTrimBegin = wxCursor(image);

    image = wxBitmap(cursor_trim_begin_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorTrimShiftBegin = wxCursor(image);

    image = wxBitmap(cursor_trim_end_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorTrimEnd = wxCursor(image);

    image = wxBitmap(cursor_trim_end_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorTrimShiftEnd = wxCursor(image);

}

void MousePointer::init()
{
    getTimeline().SetCursor(mCursorNormal);
}
    
MousePointer::~MousePointer()
{
}

void MousePointer::set(MousePointerImage image)
{
    switch (image)
    {
    case PointerNormal:         getTimeline().SetCursor(mCursorNormal);         return;
    case PointerMoveCut:        getTimeline().SetCursor(mCursorMoveCut);        return;
    case PointerTrimBegin:      getTimeline().SetCursor(mCursorTrimBegin);      return;
    case PointerTrimShiftBegin: getTimeline().SetCursor(mCursorTrimShiftBegin); return;
    case PointerTrimEnd:        getTimeline().SetCursor(mCursorTrimEnd);        return;
    case PointerTrimShiftEnd:   getTimeline().SetCursor(mCursorTrimShiftEnd);   return;
    default:                    FATAL("Unknown image");
    }
}

PointerPositionInfo MousePointer::getInfo(wxPoint pointerposition)
{
    PointerPositionInfo info;
    info.track = model::TrackPtr();
    info.clip = model::ClipPtr();
    info.trackPosition = 0;

    // Find possible videotrack under pointer
    int top = getTimeline().getDividerPosition();
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        int bottom = top;
        top -= track->getHeight();
        if (pointerposition.y <= bottom && pointerposition.y >= top)
        {
            info.track = track;
            info.trackPosition = top;
            break;
        }
    }
    if (!info.track)
    {
        // Find possible audiotrack under pointer
        int bottom = getTimeline().getDividerPosition() + Constants::sAudioVideoDividerHeight;
        BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
        {
            int top = bottom;
            bottom += track->getHeight();
            if (pointerposition.y <= bottom && pointerposition.y >= top)
            {
                info.track = track;
                info.trackPosition = top;
                break;
            }
        }
    }

    // Find clip under pointer
    if (info.track)
    {
        info.clip = info.track->getClip(getZoom().pixelsToPts(pointerposition.x));
    }

    // Find logical position of pointer wrt clips
    if (info.clip)
    {
        // This is handled on a per-pixel and not per-pts basis. That ensures
        // that this still works for clips which are very small when zoomed out.
        // (then the cursor won't flip too much).
        GuiTimeLineClip* clip = getViewMap().getView(info.clip);
        int dist_begin = pointerposition.x - clip->getLeftPosition();
        int dist_end = clip->getRightPosition() - pointerposition.x;

        if (dist_begin <= 1)
        {
            // Possibly between clips. However, this is only relevant if there
            // is another nonempty clip adjacent to this clip.
            model::ClipPtr previous = info.track->getPreviousClip(info.clip);
            info.logicalclipposition = (!previous || previous->isA<model::EmptyClip>()) ? ClipBegin : ClipBetween;
        }
        else if (dist_end <= 1)
        {
            // Possibly between clips. However, this is only relevant if there
            // is another nonempty clip adjacent to this clip.
            model::ClipPtr next = info.track->getNextClip(info.clip);
            info.logicalclipposition = (!next || next->isA<model::EmptyClip>()) ? ClipEnd : ClipBetween;
        }
        else if ((dist_begin > 1) && (dist_begin < 4))
        {
            info.logicalclipposition = ClipBegin;
        }
        else if ((dist_end > 1) && (dist_end < 4))
        {
            info.logicalclipposition = ClipEnd;
        }
        else
        {
            info.logicalclipposition = ClipInterior;
        }
    }
    return info;
}

}} // namespace
