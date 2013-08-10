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

#ifndef VIEW_UPDATE_H
#define VIEW_UPDATE_H

#include "UtilEvent.h"

namespace gui { namespace timeline {

class View;

class ViewUpdate
{
public:
    ViewUpdate(View& view, wxRegion area);
    ~ViewUpdate();
    ViewUpdate(const ViewUpdate& other);
    View& getView();
    wxRegion getArea();
private:
    View* mView;    // Stored as pointer, not reference for more info during debugging in IDE
    wxRegion mArea;
    friend std::ostream& operator<<( std::ostream& os, const ViewUpdate& obj );
};

DECLARE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

}} // namespace

#endif // VIEW_UPDATE_H