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
#include "Divider.h"
#include "AudioView.h"
#include "Timeline.h"
#include "UtilLogWxwidgets.h"
#include "Zoom.h"
#include "ViewMap.h"
#include "TrackView.h"
#include "PositionInfo.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MousePointerImage);
IMPLEMENTENUM(MouseOnClipPosition);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MousePointer::MousePointer(Timeline* timeline)
:   Part(timeline)
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

    getDivider().getPositionInfo(pointerposition, info);
    if (!info.onAudioVideoDivider)
    {
        /** @todo move to timeline class (it contains the if below) */
        getTimeline().getVideo().getPositionInfo(pointerposition, info);
        if (!info.track)
        {
            getTimeline().getAudio().getPositionInfo(pointerposition, info);
        }

        // Find clip under pointer
        if (info.track)
        {
            getViewMap().getView(info.track)->getPositionInfo(pointerposition, info);
        }
    }

    return info;
}

}} // namespace
