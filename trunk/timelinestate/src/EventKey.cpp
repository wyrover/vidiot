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

#include "EventKey.h"

namespace gui { namespace timeline { namespace state {

EvKey::EvKey(wxKeyEvent& wxEvent)
    : CtrlDown(wxEvent.ControlDown())
    , ShiftDown(wxEvent.ShiftDown())
    , AltDown(wxEvent.AltDown())
    , KeyCode(wxEvent.GetKeyCode())
    , mEvent(wxEvent)
{
};

EvKey::EvKey(wxMouseState& state, int key)
    : CtrlDown(state.ControlDown())
    , ShiftDown(state.ShiftDown())
    , AltDown(state.AltDown())
    , KeyCode(key)
    , mEvent(boost::none)
{
}

void EvKey::consumed() const
{
    if (mEvent)
    {
        mEvent->Skip(false);
    }
}

std::ostream& operator<<(std::ostream& os, const EvKey& obj)
{
    os  << typeid(obj).name()   << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.CtrlDown         << '|'
        << obj.ShiftDown        << '|'
        << obj.AltDown          << '|'
        << obj.KeyCode          << '|'
        << obj.mEvent;
    return os;
}

}}} // namespace
