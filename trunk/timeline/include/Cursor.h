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

#ifndef CURSOR_H
#define CURSOR_H

#include "Part.h"
#include "UtilInt.h"

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

    void prevFrame();
    void nextFrame();
    void prevCut();
    void nextCut();
    void home();
    void end();

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc) const;

private:

    pts mCursorPosition;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPlaybackPosition(PlaybackPositionEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Move cursor but don't update player
    /// \param position new position in sequence where cursor must be positioned
    void moveTo(pts position);

    void ensureCursorVisible();

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

#endif // CURSOR_H