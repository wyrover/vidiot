// Copyright 2013-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Mouse.h"

#include "AudioView.h"
#include "Clip.h"
#include "ClipView.h"
#include "Config.h"
#include "PositionInfo.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "StatusBar.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackView.h"
#include "UtilWindow.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "Zoom.h"
#include <wx/cursor.h>

namespace gui { namespace timeline {

IMPLEMENTENUM(MousePointerImage);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Mouse::Mouse(Timeline* timeline)
    :   Part(timeline)
    ,   mPhysicalPosition(-1,-1)
    ,   mVirtualPosition(-1,-1)
    ,   mLeftDownPosition(-1,-1)
    ,   mRightDownPosition(-1,-1)
    ,   mRightUpPosition(-1,-1)
    ,   mLeftDown(false)
    ,   mRightDown(false)
{
    VAR_DEBUG(this);

    wxImage image;

    image = util::window::getImage("cursor_normal.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorNormal = wxCursor(image);

    image = util::window::getImage("cursor_move_cut.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorMoveCut = wxCursor(image);

    image = util::window::getImage("cursor_trim_begin.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimBegin = wxCursor(image);

    image = util::window::getImage("cursor_trim_begin.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimShiftBegin = wxCursor(image);

    image = util::window::getImage("cursor_trim_end.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimEnd = wxCursor(image);

    image = util::window::getImage("cursor_trim_end.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    mCursorTrimShiftEnd = wxCursor(image);

    image = util::window::getImage("cursor_track_resize.png");
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 8);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 8);
    mCursorTrackResize = wxCursor(image);

    set(PointerNormal);
}

Mouse::~Mouse()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Mouse::update(state::EvMouse& state)
{
    mPhysicalPosition =  state.Position;
    mPhysicalPosition.x = std::max(0, mPhysicalPosition.x);                        // Some events may have positions
    mPhysicalPosition.x = std::min(getSequenceView().getW(), mPhysicalPosition.x); // outside the sequence view
    mPhysicalPosition.y = std::max(0, mPhysicalPosition.y);                        // region.
    mPhysicalPosition.y = std::min(getSequenceView().getH(), mPhysicalPosition.y); // Example: leave event.

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
        StatusBar::get().setDebugText( wxString::Format("POS:(%3d,%3d) PTS:[%5" PRId64 "] CLIP:[%5" PRId64 ",%5" PRId64 ")", mVirtualPosition.x, mVirtualPosition.y, getZoom().pixelsToPts(mVirtualPosition.x), left, right) );
    }
}

void Mouse::dragMove(const wxPoint& position)
{
    mPhysicalPosition = position;
    mVirtualPosition =  getScrolling().getVirtualPosition(mPhysicalPosition);
    VAR_DEBUG(mPhysicalPosition)(mVirtualPosition);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Mouse::set(const MousePointerImage& image)
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

PointerPositionInfo Mouse::getInfo(const wxPoint& pointerposition)
{
    PointerPositionInfo info;
    getSequenceView().getPositionInfo(pointerposition, info);
    return info;
}

wxPoint Mouse::getPhysicalPosition() const
{
    return mPhysicalPosition;
}

wxPoint Mouse::getVirtualPosition() const
{
    return mVirtualPosition;
}

wxPoint Mouse::getLeftDownPosition() const
{
    return mLeftDownPosition;
}

wxPoint Mouse::getRightDownPosition() const
{
    return mRightDownPosition;
}

wxPoint Mouse::getRightUpPosition() const
{
    return mRightUpPosition;
}

void Mouse::setLeftDown(bool down)
{
    VAR_DEBUG(down);
    if (down)
    {
        mLeftDownPosition = mVirtualPosition;
    }
    mLeftDown = down;
}

void Mouse::setRightDown(bool down)
{
    VAR_DEBUG(down);
    if (down)
    {
        mRightDownPosition = mVirtualPosition;
    }
    else
    {
        mRightUpPosition = mVirtualPosition;
    }
    mRightDown = down;
}

bool Mouse::getLeftDown() const
{
    return mLeftDown;
}

bool Mouse::getRightDown() const
{
    return mRightDown;
}


}} // namespace
