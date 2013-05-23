#include "StateMoveTrackDivider.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "ClipView.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventPart.h"
#include "EventMouse.h"
#include "Layout.h"
#include "Mouse.h"
#include "PositionInfo.h"
#include "SequenceView.h"
#include "StateIdle.h"
#include "ThumbnailView.h"
#include "Tooltip.h"
#include "Track.h"

#include "UtilLog.h"
#include "VideoClip.h"
#include "ViewMap.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MoveTrackDivider::MoveTrackDivider( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mOriginalHeight(0)
,   mTrack()
{
    LOG_DEBUG;

    PointerPositionInfo info = getMouse().getInfo(getMouse().getLeftDownPosition());
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
    BOOST_FOREACH( model::IClipPtr clip, mTrack->getClips() )
    {
        if (clip->isA<model::VideoClip>())
        {
            // Invalidating the thumbnail will also invalidate the clip itselves
            // Hence, this small optimization (do not invalidate the clip also)
            getViewMap().getThumbnail(clip)->invalidateBitmap();
        }
        else
        {
            getViewMap().getView(clip)->invalidateBitmap();
        }
    }
    return transit<Idle>();
}

boost::statechart::result MoveTrackDivider::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    int diff = getMouse().getLeftDownPosition().y - getMouse().getVirtualPosition().y;
    if (mTrack->isA<model::AudioTrack>())
    {
        // Reverse for audio tracks (these are dragged at the bottom, video tracks are dragged at the top)
        diff *= -1;
    }
    int height = mOriginalHeight + diff;
    if (height >= Layout::MinTrackHeight && height <= Layout::MaxTrackHeight)
    {
        VAR_DEBUG(height);
        mTrack->setHeight(height);
        getSequenceView().resetDividerPosition(); // Ensure that 'minimal grey above videotracks' is maintained.
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
    switch (evt.getKeyCode())
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