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

#include "StateMoveKeyFrame.h"

#include "AClipEdit.h"
#include "ClipInterval.h"
#include "ClipView.h"
#include "CommandProcessor.h"
#include "EditClipDetails.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Mouse.h"
#include "StateIdle.h"
#include "ViewMap.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MoveKeyFrame::MoveKeyFrame( my_context ctx ) // entry
    : TimeLineState(ctx)
    , mBoundaries{ 0,0 }
{
    LOG_DEBUG;

    PointerPositionInfo info{ getMouse().getInfo(getMouse().getLeftDownPosition()) };
    ASSERT(info.keyframe);

    mClip = info.getLogicalClip();
    mKeyFrameIndex = *info.keyframe;

    model::ClipIntervalPtr interval{ boost::dynamic_pointer_cast<model::ClipInterval>(mClip) };
    ASSERT_NONZERO(interval);
    mBoundaries = interval->getKeyFrameBoundaries(mKeyFrameIndex);
    mKeyFramePosition = interval->getKeyFramePosition(mKeyFrameIndex);
}

MoveKeyFrame::~MoveKeyFrame() // exit
{
    LOG_DEBUG;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveKeyFrame::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result MoveKeyFrame::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    if (mEditCommand == nullptr)
    {
        std::pair<model::IClipPtr, model::IClipPtr> clones{ cmd::AClipEdit::clone(mClip) };
        mClone = clones.first;
        mEditCommand = new cmd::EditClipDetails(getSequence(), _("Move key frame"), mClip, mClone);
        mEditCommand->submit();
        ASSERT_NONZERO(mEditCommand);
    }

    model::ClipIntervalPtr interval{ boost::dynamic_pointer_cast<model::ClipInterval>(mClone) };
    ASSERT_NONZERO(interval);
    pixel diff{ getMouse().getVirtualPosition().x - getMouse().getLeftDownPosition().x };
    pts diffPts{ getZoom().pixelsToPts(diff) };
    pts newPos{ mKeyFramePosition + diffPts };
    if (newPos < mBoundaries.first) { newPos = mBoundaries.first; }
    if (newPos > mBoundaries.second) { newPos = mBoundaries.second; }
    interval->setKeyFramePosition(mKeyFrameIndex, newPos);
    // Ensure the details view is updated.
    // Otherwise, the buttons enabling/disabling in details view is not in 
    // sync with the currently shown frame.
    getCursor().setLogicalPosition(interval->getPerceivedLeftPts() + newPos);
    return forward_event();
}

boost::statechart::result MoveKeyFrame::react( const EvLeave& evt)
{
    VAR_DEBUG(evt);
    return abort();
}

boost::statechart::result MoveKeyFrame::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_ESCAPE:    
        evt.consumed();
        return abort();
    }
    return forward_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveKeyFrame::abort()
{
    if (mEditCommand != nullptr)
    {
        // Accepted: this leaves the command as the 'redoable' command.
        model::CommandProcessor::get().Undo();
        mEditCommand = nullptr;
    }
    return transit<Idle>();
}
}}} // namespace