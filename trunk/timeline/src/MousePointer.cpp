#include "MousePointer.h"

#include "AudioView.h"
#include "Clip.h"
#include "ClipView.h"
#include "Config.h"
#include "cursor_move_cut.xpm"
#include "cursor_normal.xpm"
#include "cursor_track_resize.xpm"
#include "cursor_trim_begin.xpm"
#include "cursor_trim_end.xpm"
#include "PositionInfo.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackView.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "StatusBar.h"
#include "Zoom.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MousePointerImage);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MousePointer::MousePointer(Timeline* timeline) // todo rename to Mouse
:   Part(timeline)
,   mPhysicalPosition(-1,-1)
,   mVirtualPosition(-1,-1)
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
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void MousePointer::update(const wxMouseState& state)
{
    mPhysicalPosition = state.GetPosition();
    mVirtualPosition =  getScrolling().getVirtualPosition(mPhysicalPosition);
    VAR_DEBUG(mPhysicalPosition)(mVirtualPosition);

    if (Config::getShowDebugInfo())
    {
        PointerPositionInfo info = getInfo(mVirtualPosition);
        pts left = 0;
        pts right = 0;
        if (info.clip)
        {
            left = info.clip->getLeftPts();
            right = info.clip->getRightPts();
        }
        gui::StatusBar::get().setDebugText( wxString::Format("POS:(%3d,%3d) PTS:[%5d] CLIP:[%5ld,%5ld)", mVirtualPosition.x, mVirtualPosition.y, getZoom().pixelsToPts(mVirtualPosition.x), left, right) );
    }
}

void MousePointer::leftDown()
{
    LOG_DEBUG;
    mLeft = mVirtualPosition;
}

void MousePointer::rightDown()
{
    LOG_DEBUG;
    mRight = mVirtualPosition;
}

void MousePointer::dragMove(wxPoint position)
{
    mPhysicalPosition = position;
    mVirtualPosition =  getScrolling().getVirtualPosition(mPhysicalPosition);
    VAR_DEBUG(mPhysicalPosition)(mVirtualPosition);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

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
    case PointerTrackResize:    getTimeline().SetCursor(mCursorTrackResize);    return;
    default:                    FATAL("Unknown image");
    }
}

PointerPositionInfo MousePointer::getInfo(wxPoint pointerposition)
{
    PointerPositionInfo info;
    getSequenceView().getPositionInfo(pointerposition, info);
    return info;
}

wxPoint MousePointer::getPhysicalPosition() const
{
    return mPhysicalPosition;
}

wxPoint MousePointer::getVirtualPosition() const
{
    return mVirtualPosition;
}

wxPoint MousePointer::getLeftDownPosition() const
{
    return mLeft;
}

wxPoint MousePointer::getRightDownPosition() const
{
    return mRight;
}

}} // namespace