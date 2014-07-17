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

#ifndef TIMELINE_EVENT_MOUSE_H
#define TIMELINE_EVENT_MOUSE_H

namespace gui { namespace timeline { namespace state {

struct MouseState
{
    explicit MouseState(const wxMouseState& wx);

    const wxPoint Position;
    const bool LeftIsDown;
    const bool RightIsDown;

private:
    MouseState();

    friend std::ostream& operator<<(std::ostream& os, const MouseState& obj);
};

template< class MostDerived > // todo remove this template, similar to keyevent
struct EvMouse 
    : boost::statechart::event< MostDerived >
    , public MouseState
{
    explicit EvMouse(const wxMouseState& wx)
        : MouseState(wx)
    {
    }
    friend std::ostream& operator<<(std::ostream& os, const EvMouse& obj)
    {
        // This typeid is required to distinguish the various 'react' methods
        os  << typeid(obj).name() << static_cast<const MouseState&>(obj); 
        return os;
    }
};

struct EvMotion         : EvMouse< EvMotion >          { explicit EvMotion(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvLeftDown       : EvMouse< EvLeftDown >        { explicit EvLeftDown(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvLeftUp         : EvMouse< EvLeftUp >          { explicit EvLeftUp(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvLeftDouble     : EvMouse< EvLeftDouble >      { explicit EvLeftDouble(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvMiddleDown     : EvMouse< EvMiddleDown >      { explicit EvMiddleDown(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvMiddleUp       : EvMouse< EvMiddleUp >        { explicit EvMiddleUp(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvMiddleDouble   : EvMouse< EvMiddleDouble >    { explicit EvMiddleDouble(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvRightDown      : EvMouse< EvRightDown >       { explicit EvRightDown(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvRightUp        : EvMouse< EvRightUp >         { explicit EvRightUp(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvRightDouble    : EvMouse< EvRightDouble >     { explicit EvRightDouble(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvEnter          : EvMouse< EvEnter >           { explicit EvEnter(const wxMouseState& wx) : EvMouse(wx) {} };
struct EvLeave          : EvMouse< EvLeave >           { explicit EvLeave(const wxMouseState& wx) : EvMouse(wx) {} }; 

}}} // namespace

#endif
