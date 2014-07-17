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
    explicit EvKey(wxKeyEvent& event);

    explicit EvKey(wxMouseState& state);
    

    /// Indicates that the event has been handled by the state machine and
    /// should not propagate further upwards the event handling chain.
    void consumed() const;

    const bool CtrlDown;
    const bool ShiftDown;
    const bool AltDown;
    const int KeyCode;

private:

    boost::optional<wxKeyEvent&> mEvent;

    friend std::ostream& operator<<(std::ostream& os, const EvKey& obj);
};

struct EvKeyDown
    : public EvKey
    , public boost::statechart::event< EvKeyDown >
{
    explicit EvKeyDown(wxKeyEvent& event);
};

struct EvKeyUp
    : public EvKey
    , public boost::statechart::event< EvKeyUp >
{
    explicit EvKeyUp(wxKeyEvent& event);
};

}}} // namespace

#endif
