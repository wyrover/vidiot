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

#include "EventKey.h"

#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline { namespace state {

EvKey::EvKey(wxKeyEvent& wxEvent, const wxPoint& virtualPosition)
    :   mEvent(wxEvent)
    ,   mVirtualPosition(virtualPosition)
{
};

wxKeyEvent& EvKey::getWxEvent() const
{
    return mEvent;
}

bool EvKey::getCtrlDown() const
{
    return mEvent.ControlDown();
}

bool EvKey::getShiftDown() const
{
    return mEvent.ShiftDown();
}

bool EvKey::getAltDown() const
{
    return mEvent.AltDown();
}

bool EvKey::hasUnicodeKey() const
{
    return mEvent.GetUnicodeKey() != WXK_NONE;
}

wxChar EvKey::getUnicodeKey() const
{
    ASSERT(hasUnicodeKey());
    return mEvent.GetUnicodeKey();
}

int EvKey::getKeyCode() const
{
    return mEvent.GetKeyCode();
}

wxPoint EvKey::getPosition() const
{
    return mVirtualPosition;
}

std::ostream& operator<<(std::ostream& os, const EvKey& obj)
{
    os  << typeid(obj).name()   << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mEvent           << '|'
        << obj.mVirtualPosition;
    return os;
}

EvKeyDown::EvKeyDown(wxKeyEvent& event, const wxPoint& virtualPosition)
    : EvKey(event, virtualPosition)
{
}

EvKeyUp::EvKeyUp(wxKeyEvent& event, const wxPoint& virtualPosition)
    : EvKey(event, virtualPosition)
{
}

}}} // namespace