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

#include "Tooltip.h"

#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

Tooltip::Tooltip(Timeline* timeline)
:   Part(timeline)
{
    VAR_DEBUG(this);
    wxToolTip::SetDelay(0);
}

Tooltip::~Tooltip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Tooltip::show( const wxString& text )
{
    VAR_INFO(text);
    if (getTimeline().GetToolTipText().IsSameAs(text) || text.IsSameAs(""))
    {
        getTimeline().UnsetToolTip();
    }
    else
    {
        getTimeline().SetToolTip(text);
    }
}

}} // namespace