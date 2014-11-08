// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef MOUSE_POINTER_H
#define MOUSE_POINTER_H

#include "EventMouse.h"
#include "UtilEnum.h"
#include "Part.h"
#include "UtilInt.h"

class wxCursor;

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

    void update(state::EvMouse& state);
    
    void dragMove(const wxPoint& position);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void set(const MousePointerImage& image);

    /// \return info on the given position.
    PointerPositionInfo getInfo(const wxPoint& position);

    /// \return current physical (unscrolled) positionm of mouse pointer
    wxPoint getPhysicalPosition() const;

    /// \return current virtual (includes scrolling offset) position of mouse pointer
    wxPoint getVirtualPosition() const;

    /// \return last (most recent) position at which a 'left down' event was received.
    wxPoint getLeftDownPosition() const;

    /// \return last (most recent) position at which a 'right down' event was received.
    wxPoint getRightDownPosition() const;

    /// \return last (most recent) position at which a 'right up' event was received.
    wxPoint getRightUpPosition() const;

    void setLeftDown(bool down);
    void setRightDown(bool down);
    bool getLeftDown() const;
    bool getRightDown() const;

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
    wxPoint mLeftDownPosition;  ///< Most recent position at which the left button was moved down..
    wxPoint mRightDownPosition; ///< Most recent position at which the right button was moved down.
    wxPoint mRightUpPosition;   ///< Most recent position at which the right button was moved up.
    bool    mLeftDown; 
    bool    mRightDown;
};

}} // namespace

#endif
