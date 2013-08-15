// Copyright 2013 Eric Raijmakers.
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

EvKey::EvKey(
    bool controldown,
    bool shiftdown,
    bool altdown,
    wxChar unicodekey,
    int keycode,
    wxPoint position)
    :   mCtrlDown(controldown)
    ,   mShiftDown(shiftdown)
    ,   mAltDown(altdown)
    ,   mUnicodeKey(unicodekey)
    ,   mKeyCode(keycode)
    ,   mPosition(position)
{
};

bool EvKey::getCtrlDown() const
{
    return mCtrlDown;
}

bool EvKey::getShiftDown() const
{
    return mShiftDown;
}

bool EvKey::getAltDown() const
{
    return mAltDown;
}

bool EvKey::hasUnicodeKey() const
{
    return mUnicodeKey != WXK_NONE;
}

wxChar EvKey::getUnicodeKey() const
{
    ASSERT(hasUnicodeKey());
    return mUnicodeKey;
}

int EvKey::getKeyCode() const
{
    return mKeyCode;
}

wxPoint EvKey::getPosition() const
{
    return mPosition;
}

std::ostream& operator<< (std::ostream& os, const EvKey& obj)
{
    os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mCtrlDown      << '|'
        << obj.mShiftDown     << '|'
        << obj.mAltDown       << '|'
        << obj.mKeyCode       << '|'
        << obj.mUnicodeKey    << '|'
        << obj.mPosition;
    return os;
}

EvKeyDown::EvKeyDown(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position)
    : EvKey(controldown, shiftdown, altdown, unicodekey, keycode, position)
{
}

EvKeyUp::EvKeyUp(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position)
    : EvKey(controldown, shiftdown, altdown, unicodekey, keycode, position)
{
}

}}} // namespace