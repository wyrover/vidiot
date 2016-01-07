// Copyright 2013-2016 Eric Raijmakers.
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

#include "Drag_Shift.h"

#include "Timeline.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ShiftParams::ShiftParams(Timeline& timeline, pts position, pts length)
    : mTimeline(timeline)
    , mPosition(position)
    , mLength(length)
{
}

ShiftParams::ShiftParams(const ShiftParams& other)
    : mTimeline(other.mTimeline)
    , mPosition(other.mPosition)
    , mLength(other.mLength)
{
}

//////////////////////////////////////////////////////////////////////////
// COMPARISON
//////////////////////////////////////////////////////////////////////////

bool operator== ( ShiftParams const& x, ShiftParams const& y )
{
    return (x.mPosition == y.mPosition) && (x.mLength == y.mLength);
}

bool operator!= ( ShiftParams const& x, ShiftParams const& y )
{
    return (x.mPosition != y.mPosition) || (x.mLength != y.mLength);
}

//////////////////////////////////////////////////////////////////////////
// GET
//////////////////////////////////////////////////////////////////////////

pts ShiftParams::getPtsPosition() const
{
    return mPosition;
}

pts ShiftParams::getPtsLength() const
{
    return mLength;
}

pixel ShiftParams::getPixelPosition() const
{
    return mTimeline.getZoom().ptsToPixels(mPosition);
}

pixel ShiftParams::getPixelLength() const
{
    return mTimeline.getZoom().ptsToPixels(mLength);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ShiftParams& obj)
{
    os << obj.mPosition << '|' << obj.mLength;
    return os;
}

}} // namespace