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

struct EvMouse 
{
    explicit EvMouse(const wxMouseState& wx);
    
    friend std::ostream& operator<<(std::ostream& os, const EvMouse& obj);

    const wxPoint Position;
    const bool LeftIsDown;
    const bool RightIsDown;

};

struct EvMotion         
    : public EvMouse
    , public boost::statechart::event< EvMotion >
{ 
    explicit EvMotion(const wxMouseState& wx);
};

struct EvLeftDown
    : public EvMouse
    , public boost::statechart::event< EvLeftDown >
{ 
    explicit EvLeftDown(const wxMouseState& wx);
};

struct EvLeftUp
    : public EvMouse
    , public boost::statechart::event< EvLeftUp >
{ 
    explicit EvLeftUp(const wxMouseState& wx);
};

struct EvLeftDouble
    : public EvMouse
    , public boost::statechart::event< EvLeftDouble >
{ 
    explicit EvLeftDouble(const wxMouseState& wx);
};

struct EvMiddleDown
    : public EvMouse
    , public boost::statechart::event< EvMiddleDown >
{ 
    explicit EvMiddleDown(const wxMouseState& wx);
};

struct EvMiddleUp
    : public EvMouse
    , public boost::statechart::event< EvMiddleUp >
{ 
    explicit EvMiddleUp(const wxMouseState& wx);
};

struct EvMiddleDouble
    : public EvMouse
    , public boost::statechart::event< EvMiddleDouble >
{ 
    explicit EvMiddleDouble(const wxMouseState& wx);
};

struct EvRightDown
    : public EvMouse
    , public boost::statechart::event< EvRightDown >
{ 
    explicit EvRightDown(const wxMouseState& wx);
};

struct EvRightUp
    : public EvMouse
    , public boost::statechart::event< EvRightUp >
{ 
    explicit EvRightUp(const wxMouseState& wx);
};

struct EvRightDouble
    : public EvMouse
    , public boost::statechart::event< EvRightDouble >
{ 
    explicit EvRightDouble(const wxMouseState& wx);
};

struct EvEnter
    : public EvMouse
    , public boost::statechart::event< EvEnter >
{ 
    explicit EvEnter(const wxMouseState& wx);
};

struct EvLeave
    : public EvMouse
    , public boost::statechart::event< EvLeave >
{ 
    explicit EvLeave(const wxMouseState& wx);
};

}}} // namespace

#endif
