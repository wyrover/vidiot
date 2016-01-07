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

#pragma once

#include "Part.h"

namespace gui { class PlaybackPositionEvent; }

namespace gui { namespace timeline {

class Cursor
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    static const int EDGE_OFFSET;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Cursor(Timeline* timeline);
    virtual ~Cursor();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// \return position new position in sequence where cursor must is positioned
    pts getLogicalPosition() const;

    /// Move cursor AND update player position
    /// \param position new position in sequence where cursor must be positioned
    void setLogicalPosition(pts position);

    void setLogicalPositionAndKeepVisible(pts position);

    void onPlaybackPosition(pts position);

    void prevFrame();
    void nextFrame();
    void prevCut();
    void nextCut();
    void home();
    void end();

    /// Ensure that the scrolling is adjusted such that the cursor is displayed
    /// at 1/4th position (horizontally) of the widget length.
    void focus();

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const;

private:

    pts mCursorPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Move cursor but don't update player
    /// \param position new position in sequence where cursor must be positioned
    void moveTo(pts position);

    /// Ensure that the scrolling is adjusted if the cursor is moved outside the visible region by a user action
    /// \note the mechanism for adjusting the cursor during playback is specifically located elsewhere since a slightly different adjustment mechanism is used then.
    void ensureCursorVisible();

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace
