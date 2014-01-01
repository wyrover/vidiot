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

#ifndef DRAG_SHIFT_H
#define DRAG_SHIFT_H

#include "UtilInt.h"

namespace gui { namespace timeline {
/// Helper class for administering a shift during the DND operation
struct ShiftParams
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ShiftParams(pts position, pts length);
    ShiftParams(const ShiftParams& other);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mPosition;  ///< Position at which clips must be shifted to make room for the clips being dragged.
    pts mLength;    ///< Length of the shift required to make room for the clips being dragged.
};

std::ostream& operator<< (std::ostream& os, const ShiftParams& obj);
bool operator== ( ShiftParams const& x, ShiftParams const& y );
bool operator!= ( ShiftParams const& x, ShiftParams const& y );

typedef boost::optional<ShiftParams> Shift;
}} // namespace

#endif