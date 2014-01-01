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

#include "Keyboard.h"

#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Keyboard::Keyboard(Timeline* timeline)
:   Part(timeline)
,   mCtrlDown(false)
,   mShiftDown(false)
,   mAltDown(false)
{
    VAR_DEBUG(this);
}

Keyboard::~Keyboard()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Keyboard::update(const wxKeyboardState& state)
{
    bool CtrlDown(state.ControlDown());
    bool ShiftDown(state.ShiftDown());
    bool AltDown(state.AltDown());
    VAR_DEBUG(CtrlDown)(ShiftDown)(AltDown);
    setCtrlDown(CtrlDown);
    setShiftDown(ShiftDown);
    setAltDown(AltDown);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Keyboard::setCtrlDown(bool down)
{
    mCtrlDown = down;
}

bool Keyboard::getCtrlDown() const
{
    return mCtrlDown;
}

void Keyboard::setShiftDown(bool down)
{
    mShiftDown = down;
}

bool Keyboard::getShiftDown() const
{
    return mShiftDown;
}

void Keyboard::setAltDown(bool down)
{
    mAltDown = down;
}

bool Keyboard::getAltDown() const
{
    return mAltDown;
}
}} // namespace