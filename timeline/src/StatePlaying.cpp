#include "StatePlaying.h"

#include "GuiTimeLine.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "GuiPlayer.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Playing::Playing( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mMakingNewSelection(false)
{
    LOG_DEBUG; 
}

Playing::~Playing() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Playing::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    post_event(evt); // Handle this event again in the Idle state
    return stop();
}

boost::statechart::result Playing::react( const EvKeyDown& evt)
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SPACE:
        return stop();
    case WXK_SHIFT:
        triggerBegin();
        break;
    }
    return discard_event();
}

boost::statechart::result Playing::react( const EvKeyUp& evt)
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        triggerEnd();
        break;
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Playing::stop()
{
    triggerEnd();
    getPlayer()->stop();
    return transit<Idle>();
}

void Playing::triggerBegin()
{
    if (!mMakingNewSelection)
    {
        getSelectIntervals().addBeginMarker();
        mMakingNewSelection = true;
    }
}

void Playing::triggerEnd()
{
    if (mMakingNewSelection)
    {
        getSelectIntervals().addEndMarker();
        mMakingNewSelection = false;
    }
}

}}} // namespace