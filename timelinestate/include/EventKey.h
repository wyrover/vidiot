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
    EvKey(wxKeyEvent& event, const wxPoint& virtualPosition);

    wxKeyEvent& getWxEvent() const;

    bool getCtrlDown() const;
    bool getShiftDown() const;
    bool getAltDown() const;
    bool hasUnicodeKey() const;
    wxChar getUnicodeKey() const;
    int getKeyCode() const;
    wxPoint getPosition() const;

private:

    wxKeyEvent& mEvent;

    wxPoint mVirtualPosition;

    friend std::ostream& operator<<(std::ostream& os, const EvKey& obj);
};

struct EvKeyDown
    : public EvKey
    , public boost::statechart::event< EvKeyDown >
{
    EvKeyDown(wxKeyEvent& event, const wxPoint& virtualPosition);
};

struct EvKeyUp
    : public EvKey
    , public boost::statechart::event< EvKeyUp >
{
    EvKeyUp(wxKeyEvent& event, const wxPoint& virtualPosition);
};

}}} // namespace

#endif
