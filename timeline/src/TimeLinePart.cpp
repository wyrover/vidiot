#include "TimeLinePart.h"

#include "GuiTimeLine.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

TimeLinePart::TimeLinePart()
:   mTimeline(0)
{
}

void TimeLinePart::initTimeline(GuiTimeLine* timeline)
{
    mTimeline = timeline;
    init();
}

void TimeLinePart::init()
{
}

TimeLinePart::~TimeLinePart()
{
}

//////////////////////////////////////////////////////////////////////////
// PARTS 
//////////////////////////////////////////////////////////////////////////

GuiTimeLine& TimeLinePart::getTimeline()
{
    ASSERT(mTimeline);
    return *mTimeline;
}
// Using the member from this point onwards will lead to compiler errors.
// Using mTimeline instead of getTimeline() will lead to problems in the
// state classes (these override getTimeline).
#define mTimeline DO_NOT_USE_MEMBER_USE_METHOD

GuiTimeLineZoom& TimeLinePart::getZoom() 
{ 
    return getTimeline().getZoom(); 
}

SelectIntervals& TimeLinePart::getSelectIntervals() 
{ 
    return getTimeline().getSelectIntervals();
}

SelectClips& TimeLinePart::getSelectClips()
{
    return getTimeline().getSelectClips();
}

MousePointer& TimeLinePart::getMousePointer()
{
    return getTimeline().getMousepointer();
}

ViewMap& TimeLinePart::getViewMap()
{
    return getTimeline().getViewMap();
}

//////////////////////////////////////////////////////////////////////////
// OTHER HELPER METHODS
//////////////////////////////////////////////////////////////////////////

PlayerPtr TimeLinePart::getPlayer()
{
    return getTimeline().getPlayer();
}

model::SequencePtr TimeLinePart::getSequence()
{
    return getTimeline().getSequence();
}

}} // namespace
