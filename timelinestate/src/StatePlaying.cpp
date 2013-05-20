#include "StatePlaying.h"

#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Intervals.h"
#include "Player.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

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
    getPlayer()->stop();
    triggerEnd();
    return transit< Idle >();
}

boost::statechart::result Playing::react( const EvRightDown& evt )
{
    VAR_DEBUG(evt);
    post_event(evt); // Handle this event again in the Idle state
    getPlayer()->stop();
    triggerEnd();
    return transit< Idle >();
}

boost::statechart::result Playing::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.getKeyCode())
    {
    case WXK_SPACE:
        getPlayer()->stop();
        break;
    case WXK_SHIFT:
        triggerBegin();
        break;
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return forward_event();
}

boost::statechart::result Playing::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    switch (evt.getKeyCode())
    {
    case WXK_SHIFT:
        triggerEnd();
        break;
    }
    return forward_event();
}

boost::statechart::result Playing::react( const EvPlaybackChanged& evt)
{
    VAR_DEBUG(evt);
    if (!evt.mActive)
    {
        triggerEnd();
        return transit< Idle >();
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Playing::triggerBegin()
{
    if (!mMakingNewSelection)
    {
        getIntervals().addBeginMarker();
        mMakingNewSelection = true;
    }
}

void Playing::triggerEnd()
{
    if (mMakingNewSelection)
    {
        getIntervals().addEndMarker();
        mMakingNewSelection = false;
    }
}

}}} // namespace