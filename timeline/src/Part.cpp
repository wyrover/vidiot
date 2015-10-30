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

#include "Part.h"

#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

Part::Part()
    :   mTimeline(0)
{
    VAR_DEBUG(this);
}

Part::Part(Timeline* timeline)
    :   mTimeline(timeline)
{
    VAR_DEBUG(this);
}

Part::~Part()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// PARTS
//////////////////////////////////////////////////////////////////////////

Timeline& Part::getTimeline()
{
    ASSERT(mTimeline);
    return *mTimeline;
}

const Timeline& Part::getTimeline() const
{
    return *mTimeline;
}

// Using the member from this point onwards will lead to compiler errors.
// Using mTimeline instead of getTimeline() will lead to problems in the
// state classes (these override getTimeline).
#define mTimeline DO_NOT_USE_MEMBER_USE_METHOD

SequenceView& Part::getSequenceView()
{
    return getTimeline().getSequenceView();
}

const SequenceView& Part::getSequenceView() const
{
    return getTimeline().getSequenceView();
}

Zoom& Part::getZoom()
{
    return getTimeline().getZoom();
}

const Zoom& Part::getZoom() const
{
    return getTimeline().getZoom();
}

Intervals& Part::getIntervals()
{
    return getTimeline().getIntervals();
}

const Intervals& Part::getIntervals() const
{
    return getTimeline().getIntervals();
}

Keyboard& Part::getKeyboard()
{
    return getTimeline().getKeyboard();
}

const Keyboard& Part::getKeyboard() const
{
    return getTimeline().getKeyboard();
}

TimelineClipboard& Part::getClipboard()
{
    return getTimeline().getClipboard();
}

const TimelineClipboard& Part::getClipboard() const
{
    return getTimeline().getClipboard();
}

Selection& Part::getSelection()
{
    return getTimeline().getSelection();
}

const Selection& Part::getSelection() const
{
    return getTimeline().getSelection();
}

Mouse& Part::getMouse()
{
    return getTimeline().getMouse();
}

const Mouse& Part::getMouse() const
{
    return getTimeline().getMouse();
}

Scrolling& Part::getScrolling()
{
    return getTimeline().getScrolling();
}

const Scrolling& Part::getScrolling() const
{
    return getTimeline().getScrolling();
}

ViewMap& Part::getViewMap()
{
    return getTimeline().getViewMap();
}

const ViewMap& Part::getViewMap() const
{
    return getTimeline().getViewMap();
}

MenuHandler& Part::getMenuHandler()
{
    return getTimeline().getMenuHandler();
}

const MenuHandler& Part::getMenuHandler() const
{
    return getTimeline().getMenuHandler();
}

Cursor& Part::getCursor()
{
    return getTimeline().getCursor();
}

const Cursor& Part::getCursor() const
{
    return getTimeline().getCursor();
}

Drag& Part::getDrag()
{
    return getTimeline().getDrag();
}

const Drag& Part::getDrag() const
{
    return getTimeline().getDrag();
}

Trim& Part::getTrim()
{
    return getTimeline().getTrim();
}

const Trim& Part::getTrim() const
{
    return getTimeline().getTrim();
}

state::Machine& Part::getStateMachine()
{
    return getTimeline().getStateMachine();
}

const state::Machine& Part::getStateMachine() const
{
    return getTimeline().getStateMachine();
}

Details& Part::getDetails()
{
    return getTimeline().getDetails();
}

const Details& Part::getDetails() const
{
    return getTimeline().getDetails();
}

//////////////////////////////////////////////////////////////////////////
// OTHER HELPER METHODS
//////////////////////////////////////////////////////////////////////////

Player* Part::getPlayer()
{
    return getTimeline().getPlayer();
}

model::SequencePtr Part::getSequence()
{
    return getTimeline().getSequence();
}

const model::SequencePtr Part::getSequence() const
{
    return getTimeline().getSequence();
}

}} // namespace