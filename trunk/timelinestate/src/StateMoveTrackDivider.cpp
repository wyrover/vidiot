#include "StateMoveTrackDivider.h"

#include <boost/foreach.hpp>
#include "StateIdle.h"
#include "UtilLog.h"
#include "MousePointer.h"
#include "ViewMap.h"
#include "PositionInfo.h"
#include "Track.h"
#include "Layout.h"
#include "Divider.h"
#include "ClipView.h"
#include "AudioTrack.h"
#include "Tooltip.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MoveTrackDivider::MoveTrackDivider( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mStartPosition(0,0)
,   mOriginalHeight(0)
,   mTrack()
{
    LOG_DEBUG;

    const EvLeftDown* event = dynamic_cast<const EvLeftDown*>(triggering_event());
    ASSERT(event); // Only way to get here is to press left button in the Idle state

    mStartPosition = event->mPosition;

    PointerPositionInfo info = getMousePointer().getInfo(mStartPosition);
    ASSERT(info.onTrackDivider);

    mTrack = info.track;
    mOriginalHeight = mTrack->getHeight();
}

MoveTrackDivider::~MoveTrackDivider() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveTrackDivider::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    FATAL("Unexpected event.");
    return forward_event();
}

boost::statechart::result MoveTrackDivider::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    BOOST_FOREACH( model::ClipPtr clip, mTrack->getClips() )
    {
        getViewMap().getView(clip)->updateThumbnail();
    }
    return transit<Idle>();
}

boost::statechart::result MoveTrackDivider::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    int diff = mStartPosition.y - evt.mPosition.y;
    if (mTrack->isA<model::AudioTrack>())
    {
        // Reverse for audio tracks (these are dragged at the bottom, video tracks are dragged at the top)
        diff *= -1;
    }
    int height = mOriginalHeight + diff;
    if (height >= Layout::sMinTrackHeight && height <= Layout::sMaxTrackHeight)
    {
        VAR_DEBUG(height);
        mTrack->setHeight(height);
        getDivider().resetPosition(); // Ensure that 'minimal grey above videotracks' is maintained.
    }
    return forward_event();
}

boost::statechart::result MoveTrackDivider::react( const EvLeave& evt)
{
    VAR_DEBUG(evt);
    return abort();
}

boost::statechart::result MoveTrackDivider::react( const EvKeyDown& evt)
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

boost::statechart::result MoveTrackDivider::abort()
{
    if (mTrack->getHeight() != mOriginalHeight)
    {
        mTrack->setHeight(mOriginalHeight);
    }
    return transit<Idle>();
}
}}} // namespace