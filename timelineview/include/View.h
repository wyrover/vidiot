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

#include "Part.h"

namespace gui { namespace timeline {

class View
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /// For initializing the parent view for the dragged track view
    View(Timeline* timeline);

    /// For initializing child views.
    View(View* parent);

    virtual ~View();

    //////////////////////////////////////////////////////////////////////////
    // POSITION/SIZE
    //////////////////////////////////////////////////////////////////////////

    virtual pixel getX() const = 0;
    virtual pixel getY() const = 0;
    virtual pixel getW() const = 0;
    virtual pixel getH() const = 0;

    wxPoint getPosition() const;
    wxSize getSize() const;
    wxRect getRect() const;

    /// Must be called whenever cached values are no longer valid
    /// (typically boost::optionals containing width and/or height).
    virtual void invalidateRect() = 0;

    /// Trigger a refresh of the area (of the timeline) covered by this View
    void repaint();

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    /// \param dc draw onto this dc
    /// \param region only this region must be (re)drawn
    /// \param scroll offset to be applied because of scrolling
    virtual void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const = 0;

protected:

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    View& getParent() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    View* mParent;

};

}} // namespace
