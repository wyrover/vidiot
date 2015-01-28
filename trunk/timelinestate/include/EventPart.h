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

#ifndef TIMELINE_EVENT_PART_H
#define TIMELINE_EVENT_PART_H

#include "Part.h"
#include "Zoom.h"
#include "Scrolling.h"

namespace gui { namespace timeline { namespace state {

template< class MostDerived, class ORIGINALEVENT >
struct EvPart : boost::statechart::event< MostDerived >
{
    explicit EvPart(ORIGINALEVENT& event)
        :   mEvent(event)
    {
    };
    const ORIGINALEVENT mEvent;
};

template< class MostDerived, class ORIGINALEVENT >
std::ostream& operator<<(std::ostream& os, const EvPart< MostDerived, ORIGINALEVENT >& obj)
{
    os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mEvent;
    return os;
}

struct EvPlaybackChanged
    : boost::statechart::event< EvPlaybackChanged >
{
    EvPlaybackChanged(bool active)
        : mActive(active)
    {}
    bool mActive;

    friend std::ostream& operator<<(std::ostream& os, const EvPlaybackChanged& obj)
    {
        os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
            << obj.mActive;
        return os;
    }
};



struct EvPlaybackPositionChanged
    : boost::statechart::event< EvPlaybackPositionChanged >
{
    EvPlaybackPositionChanged(pts position)
        : mPosition(position)
    {}
    pts mPosition;

    friend std::ostream& operator<<(std::ostream& os, const EvPlaybackPositionChanged& obj)
    {
        os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
            << obj.mPosition;
        return os;
    }
};

}}} // namespace

#endif
