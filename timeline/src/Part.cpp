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

Selection& Part::getSelection()
{
    return getTimeline().getSelection();
}

const Selection& Part::getSelection() const
{
    return getTimeline().getSelection();
}

MousePointer& Part::getMousePointer()
{
    return getTimeline().getMousepointer();
}

const MousePointer& Part::getMousePointer() const
{
    return getTimeline().getMousepointer();
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

Tooltip& Part::getTooltip()
{
    return getTimeline().getTooltip();
}

const Tooltip& Part::getTooltip() const
{
    return getTimeline().getTooltip();
}

Trim& Part::getTrim()
{
    return getTimeline().getTrim();
}

const Trim& Part::getTrim() const
{
    return getTimeline().getTrim();
}

Dump& Part::getDump()
{
    return getTimeline().getDump();
}

const Dump& Part::getDump() const
{
    return getTimeline().getDump();
}

state::Machine& Part::getStateMachine()
{
    return getTimeline().getStateMachine();
}

const state::Machine& Part::getStateMachine() const
{
    return getTimeline().getStateMachine();
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