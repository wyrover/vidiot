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

#include "View.h"

#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

View::View(Timeline* timeline)
:   Part(timeline)
,   mParent(0)
{
    VAR_DEBUG(this);
    ASSERT(timeline);
}

View::View(View* parent)
:   Part(&(parent->getTimeline()))
,   mParent(parent)
{
    VAR_DEBUG(this);
    ASSERT(mParent);
}

View::~View()
{
}

//////////////////////////////////////////////////////////////////////////
// POSITION/SIZE
//////////////////////////////////////////////////////////////////////////

wxPoint View::getPosition() const
{
    return wxPoint(getX(),getY());
}

wxSize View::getSize() const
{
    return wxSize(getW(),getH());
}

wxRect View::getRect() const
{
    return wxRect(getPosition(),getSize());
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

View& View::getParent() const
{
    ASSERT_NONZERO(mParent);
    return *mParent;
}

//////////////////////////////////////////////////////////////////////////
// BITMAP
//////////////////////////////////////////////////////////////////////////

void View::repaint()
{
    getTimeline().repaint(getRect());
}

}} // namespace