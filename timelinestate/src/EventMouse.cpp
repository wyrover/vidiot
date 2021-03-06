// Copyright 2014-2016 Eric Raijmakers.
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

#include "EventMouse.h"

namespace gui { namespace timeline { namespace state {

EvMouse::EvMouse(const wxMouseState& wx)
    : Position(wx.GetPosition())
    , LeftIsDown(wx.LeftIsDown())
    , RightIsDown(wx.RightIsDown())
{
}

std::ostream& operator<<(std::ostream& os, const EvMouse& obj)
{
    // This typeid is required to distinguish the various 'react' methods
    os  << typeid(obj).name() << '|'
        << obj.Position << '|' 
        << obj.LeftIsDown << '|' 
        << obj.RightIsDown;
    return os;
}

}}} // namespace
