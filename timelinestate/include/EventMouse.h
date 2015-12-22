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

#pragma once

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
    using EvMouse::EvMouse;
};

struct EvLeftDown
    : public EvMouse
    , public boost::statechart::event< EvLeftDown >
{ 
    using EvMouse::EvMouse;
};

struct EvLeftUp
    : public EvMouse
    , public boost::statechart::event< EvLeftUp >
{ 
    using EvMouse::EvMouse;
};

struct EvLeftDouble
    : public EvMouse
    , public boost::statechart::event< EvLeftDouble >
{ 
    using EvMouse::EvMouse;
};

struct EvMiddleDown
    : public EvMouse
    , public boost::statechart::event< EvMiddleDown >
{ 
    using EvMouse::EvMouse;
};

struct EvMiddleUp
    : public EvMouse
    , public boost::statechart::event< EvMiddleUp >
{ 
    using EvMouse::EvMouse;
};

struct EvMiddleDouble
    : public EvMouse
    , public boost::statechart::event< EvMiddleDouble >
{ 
    using EvMouse::EvMouse;
};

struct EvRightDown
    : public EvMouse
    , public boost::statechart::event< EvRightDown >
{ 
    using EvMouse::EvMouse;
};

struct EvRightUp
    : public EvMouse
    , public boost::statechart::event< EvRightUp >
{ 
    using EvMouse::EvMouse;
};

struct EvRightDouble
    : public EvMouse
    , public boost::statechart::event< EvRightDouble >
{ 
    using EvMouse::EvMouse;
};

struct EvEnter
    : public EvMouse
    , public boost::statechart::event< EvEnter >
{ 
    using EvMouse::EvMouse;
};

struct EvLeave
    : public EvMouse
    , public boost::statechart::event< EvLeave >
{ 
    using EvMouse::EvMouse;
};

}}} // namespace
