#include "MousePointer.h"

#include <wx/image.h>
#include <wx/bitmap.h>
#include "Clip.h"
#include "ClipView.h"
#include "UtilLog.h"
#include "cursor_move_cut.xpm"
#include "cursor_trim_begin.xpm"
#include "cursor_trim_end.xpm"
#include "cursor_normal.xpm"
#include "cursor_track_resize.xpm"
#include "Track.h"
#include "Sequence.h"
#include "VideoView.h"
#include "AudioView.h"
#include "Timeline.h"
#include "SequenceView.h"
#include "UtilLogWxwidgets.h"
#include "Zoom.h"
#include "ViewMap.h"
#include "TrackView.h"
#include "PositionInfo.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MousePointerImage);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MousePointer::MousePointer(Timeline* timeline)
:   Part(timeline)
,   mLeft(-1,-1)
,   mRight(-1,-1)
{
    VAR_DEBUG(this);

    wxImage image;

    image = wxBitmap(cursor_normal_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorNormal = wxCursor(image);

    image = wxBitmap(cursor_move_cut_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorMoveCut = wxCursor(image);

    image = wxBitmap(cursor_trim_begin_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimBegin = wxCursor(image);

    image = wxBitmap(cursor_trim_begin_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimShiftBegin = wxCursor(image);

    image = wxBitmap(cursor_trim_end_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimEnd = wxCursor(image);

    image = wxBitmap(cursor_trim_end_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimShiftEnd = wxCursor(image);

    image = wxBitmap(cursor_track_resize_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 8);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 8);
    mCursorTrackResize = wxCursor(image);

    set(PointerNormal);
}
    
MousePointer::~MousePointer()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void MousePointer::set(MousePointerImage image)
{
    switch (image)
    {
    case PointerNormal:         getWindow().SetCursor(mCursorNormal);         return;
    case PointerMoveCut:        getWindow().SetCursor(mCursorMoveCut);        return;
    case PointerTrimBegin:      getWindow().SetCursor(mCursorTrimBegin);      return;
    case PointerTrimShiftBegin: getWindow().SetCursor(mCursorTrimShiftBegin); return;
    case PointerTrimEnd:        getWindow().SetCursor(mCursorTrimEnd);        return;
    case PointerTrimShiftEnd:   getWindow().SetCursor(mCursorTrimShiftEnd);   return;
    case PointerTrackResize:    getWindow().SetCursor(mCursorTrackResize);    return;
    default:                    FATAL("Unknown image");
    }
}

PointerPositionInfo MousePointer::getInfo(wxPoint pointerposition)
{
    PointerPositionInfo info;
    getSequenceView().getPositionInfo(pointerposition, info);
    return info;
}

void MousePointer::setLeftDownPosition(wxPoint position)
{
    VAR_DEBUG(mLeft)(position);
    mLeft = position;
}

wxPoint MousePointer::getLeftDownPosition() const
{
    return mLeft;
}

void MousePointer::setRightDownPosition(wxPoint position)
{
    VAR_DEBUG(mRight)(position);
    mRight = position;
}

wxPoint MousePointer::getRightDownPosition() const
{
    return mRight;
}

}} // namespace
