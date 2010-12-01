#include "Part.h"

#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

Part::Part()
:   mTimeline(0)
{
}

Part::Part(Timeline* timeline)
:   mTimeline(timeline)
{
}

Part::~Part()
{
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

wxScrolledWindow& Part::getWindow()
{
    return getTimeline();
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

Selection& Part::getSelection()
{
    return getTimeline().getSelection();
}

MousePointer& Part::getMousePointer()
{
    return getTimeline().getMousepointer();
}

ViewMap& Part::getViewMap()
{
    return getTimeline().getViewMap();
}

MenuHandler& Part::getMenuHandler()
{
    return getTimeline().getMenuHandler();
}

Cursor& Part::getCursor()
{
    return getTimeline().getCursor();
}

Drag& Part::getDrag()
{
    return getTimeline().getDrag();
}

Drop& Part::getDrop()
{
    return getTimeline().getDrop();
}

//////////////////////////////////////////////////////////////////////////
// OTHER HELPER METHODS
//////////////////////////////////////////////////////////////////////////

PlayerPtr Part::getPlayer()
{
    return getTimeline().getPlayer();
}

model::SequencePtr Part::getSequence()
{
    return getTimeline().getSequence();
}

}} // namespace
