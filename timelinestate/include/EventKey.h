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

#ifndef TIMELINE_EVENT_KEY_H
#define TIMELINE_EVENT_KEY_H

namespace gui { namespace timeline { namespace state {

struct EvKey
{
    EvKey(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position);

    bool getCtrlDown() const;
    bool getShiftDown() const;
    bool getAltDown() const;
    bool hasUnicodeKey() const;
    wxChar getUnicodeKey() const;
    int getKeyCode() const;
    wxPoint getPosition() const;

private:

    const bool mCtrlDown;
    const bool mShiftDown;
    const bool mAltDown;
    const int mKeyCode;
    const wxChar mUnicodeKey;
    const wxPoint mPosition;

    friend std::ostream& operator<< (std::ostream& os, const EvKey& obj);
};

struct EvKeyDown
    : public EvKey
    , public boost::statechart::event< EvKeyDown >
{
    EvKeyDown(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position);
};

struct EvKeyUp
    : public EvKey
    , public boost::statechart::event< EvKeyUp >
{
    EvKeyUp(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position);
};

}}} // namespace

#endif
