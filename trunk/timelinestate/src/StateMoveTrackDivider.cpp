// Copyright 2013,2014 Eric Raijmakers.
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

const wxString MoveTrackDivider::sTooltip = _(
    "Move the cursor to 'scrub' over the timeline and see the frames back in the preview."
    );

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
    getViewMap().invalidateThumbnails(); // Redraw thumbnails when move is done
    getTimeline().Refresh(false);
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
    case WXK_ESCAPE:    return abort();
    case WXK_F1:        getTooltip().show(sTooltip); break;
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