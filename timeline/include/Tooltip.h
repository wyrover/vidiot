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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "Part.h"

namespace gui { namespace timeline {

class Tooltip
    :   public Part
{
public:
    Tooltip(Timeline* timeline);
    virtual ~Tooltip();

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    /// Shows or hides the current tooltip.
    /// If the given text equals the current tooltip -> hide
    /// If the given text equals "" -> hide
    /// In all other cases -> show new tooltip containing text
    /// \param tooltip to be shown or hidden
    void show( const wxString& text );
};

}} // namespace

#endif
