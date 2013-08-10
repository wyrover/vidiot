// Copyright 2013 Eric Raijmakers.
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

#include "ViewUpdateEvent.h"

#include "View.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline {

ViewUpdate::ViewUpdate(View& view, wxRegion area)
    :   mView(&view)
    ,   mArea(area)
{
}

ViewUpdate::~ViewUpdate()
{
}

ViewUpdate::ViewUpdate(const ViewUpdate& other)
    :   mView(other.mView)
    ,   mArea(other.mArea)
{
}

View& ViewUpdate::getView()
{
    return *mView;
}

wxRegion ViewUpdate::getArea()
{
    return mArea;
}

std::ostream& operator<<( std::ostream& os, const ViewUpdate& obj )
{
    os << obj.mView << '|' << obj.mArea;
    return os;
}

DEFINE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

}} // namespace
