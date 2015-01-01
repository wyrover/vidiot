// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "EventKey.h"
#include "Part.h"

namespace gui { namespace timeline {

class Keyboard
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Keyboard(Timeline* timeline);
    virtual ~Keyboard();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void update(const state::EvKey& event);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //// //////////////////////////////////////////////////////////////////////

    void setCtrlDown(bool down);
    bool getCtrlDown() const;

    void setShiftDown(bool down);
    bool getShiftDown() const;

    void setAltDown(bool down);
    bool getAltDown() const;

private:

    bool mCtrlDown;
    bool mShiftDown;
    bool mAltDown;
};

}} // namespace

#endif
