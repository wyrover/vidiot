#include "StateMoveDivider.h"

#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Sequence.h"
#include "MousePointer.h"
#include "SequenceView.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MoveDivider::MoveDivider( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mOriginalPosition(getSequence()->getDividerPosition())
,   mStartPosition(getMousePointer().getLeftDownPosition())
{
    LOG_DEBUG;
}

MoveDivider::~MoveDivider() // exit
{
    LOG_DEBUG;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveDivider::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result MoveDivider::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    pixel position = mOriginalPosition + (evt.mPosition.y - mStartPosition.y);
    getSequenceView().setDividerPosition(position);
    return forward_event();
}

boost::statechart::result MoveDivider::react( const EvLeave& evt)
{
    VAR_DEBUG(evt);
    return abort();
}

boost::statechart::result MoveDivider::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_ESCAPE:
        return abort();
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return forward_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveDivider::abort()
{
    getSequenceView().setDividerPosition(mOriginalPosition);
    return transit<Idle>();
}
}}} // namespace