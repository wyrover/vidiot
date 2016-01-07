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

#pragma once

namespace gui { namespace timeline { namespace state {

template< class MostDerived >
struct EvClipboard : boost::statechart::event< MostDerived >
{
    friend std::ostream& operator<<(std::ostream& os, const EvClipboard& obj)
    {
        os  << typeid(obj).name(); // This typeid is required to distinguish the various 'react' methods
        return os;
    }
};

struct EvCut : EvClipboard< EvCut > {};
struct EvCopy : EvClipboard< EvCopy > {};
struct EvPaste : EvClipboard< EvPaste > {};

}}} // namespace
