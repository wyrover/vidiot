#include "StateTrimBegin.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/cmdproc.h>
#include <wx/wupdlock.h>
#include <boost/foreach.hpp>
#include <boost/limits.hpp>
#include <boost/make_shared.hpp>
#include "StateIdle.h"
#include "UtilLog.h"
#include "Tooltip.h"
#include "Clip.h"
#include "UtilCloneable.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "GuiPlayer.h"
#include "EditDisplay.h"
#include "Sequence.h"
#include "Scrolling.h"
#include "Track.h"
#include "VideoClip.h"
#include "ClipView.h"
#include "Project.h"
#include "Zoom.h"
#include "Timeline.h"
#include "TrimBegin.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _(
    "Move the cursor to change the entry point of the clip.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimBegin::TrimBegin( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mStartPosition(0,0)
    ,   mEdit(0)
    ,   mOriginalClip()
    ,   mMinShiftOtherTrackContent((std::numeric_limits<pts>::min)())
    ,   mMaxShiftOtherTrackContent((std::numeric_limits<pts>::max)())
    ,   mMustUndo(false)
    ,   mShiftDown(false)
    ,   mFixedPixel(0)
{
    LOG_DEBUG; 

    const EvLeftDown* event = dynamic_cast<const EvLeftDown*>(triggering_event());
    ASSERT(event); // Only way to get here is to press left button in the Idle state

    // Determine if pointer was at begin or at end of clip
    PointerPositionInfo info = getMousePointer().getInfo(event->mPosition);
    ASSERT(info.clip && !info.clip->isA<model::EmptyClip>())(info);
    mTrimBegin = (info.logicalclipposition == ClipBegin);

    mShiftDown = event->mWxEvent.ShiftDown();

    // \todo use the leftmost of the clip and/or its link
    mStartPosition = event->mWxEvent.GetPosition();
    mCurrentPosition = mStartPosition;
    mOriginalClip = info.clip;
    if (mTrimBegin)
    {
        mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clip's are added/removed, that's very volatile information).
    }
    else
    {
        mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clip's are added/removed, that's very volatile information).
    }
    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.

    // Determine boundaries for shifting other tracks
    // TODO more testing
    model::ClipPtr linked = mOriginalClip->getLink();
    pts shiftFrom = mOriginalClip->getLeftPts();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        if (mOriginalClip->getTrack() == track) continue;
        if (linked && linked->getTrack() == track) continue;
        model::ClipPtr clipAt = track->getClip(shiftFrom);
        mMinShiftOtherTrackContent = 
            (clipAt->isA<model::EmptyClip>()) ? std::max<pts>(mMinShiftOtherTrackContent, clipAt->getLeftPts() - shiftFrom) : 0;
        mMaxShiftOtherTrackContent = 
            (clipAt->isA<model::EmptyClip>()) ? std::min<pts>(mMaxShiftOtherTrackContent, clipAt->getRightPts() - shiftFrom) : 0;
    }

    mEdit = getPlayer()->startEdit();
    show();
}

TrimBegin::~TrimBegin() // exit
{
    getPlayer()->endEdit();
    LOG_DEBUG; 
}
//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result TrimBegin::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result TrimBegin::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    if (mCurrentPosition != evt.mWxEvent.GetPosition())
    {
        mCurrentPosition = evt.mWxEvent.GetPosition();
        show();
    }
    return forward_event();
}

boost::statechart::result TrimBegin::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    if (mShiftDown != evt.mWxEvent.ShiftDown())
    {
        mShiftDown = evt.mWxEvent.ShiftDown();
        show();
    }
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return forward_event();
}

boost::statechart::result TrimBegin::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    if (mShiftDown != evt.mWxEvent.ShiftDown())
    {
        mShiftDown = evt.mWxEvent.ShiftDown();
        show();
    }
    return forward_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pts TrimBegin::getLeftEmptyArea(model::ClipPtr clip)
{
    model::TrackPtr track = clip->getTrack();
    pts leftmost = clip->getLeftPts();
    model::ClipPtr previous = track->getPreviousClip(clip);
    while (previous && previous->isA<model::EmptyClip>())
    {
        leftmost = previous->getLeftPts();
        previous = track->getPreviousClip(previous);
    }
    return leftmost - clip->getLeftPts();
}

pts TrimBegin::getRightEmptyArea(model::ClipPtr clip)
{
    model::TrackPtr track = clip->getTrack();
    pts rightmost = clip->getRightPts();
    model::ClipPtr next = track->getNextClip(clip);
    while (next && next->isA<model::EmptyClip>())
    {
        rightmost = next->getRightPts();
        next = track->getNextClip(next);
    }
    return rightmost - clip->getRightPts();
}

void lowerlimit(pts& p, pts limit)
{
    if (p < limit) { p = limit; }
}

void upperlimit(pts& p, pts limit)
{
    if (p > limit) { p = limit; }
}

pts TrimBegin::getDiff()
{
    ASSERT(!mMustUndo); // If a command has been submitted, mOriginalClip can not be used.

    pts diff = getZoom().pixelsToPts(mCurrentPosition.x - mStartPosition.x);
    model::ClipPtr linked = mOriginalClip->getLink();

    if (mShiftDown)
    {
        lowerlimit(diff, mMinShiftOtherTrackContent);       // When shift trimming: the contents in other tracks must be able to be shifted accordingly
        upperlimit(diff, mMaxShiftOtherTrackContent);       // When shift trimming: the contents in other tracks must be able to be shifted accordingly
    }

    if (mTrimBegin)
    {
        upperlimit(diff, mOriginalClip->getMaxAdjustBegin());   // Clip cannot be trimmed further than the original number of frames
        upperlimit(diff, linked->getMaxAdjustBegin());          // Clip cannot be trimmed further than the original number of frames in the linked clip
        lowerlimit(diff, mOriginalClip->getMinAdjustBegin());   // Clip cannot be extended further than the '0'th frame of the underlying video provider.
        lowerlimit(diff, linked->getMinAdjustBegin());          // Link cannot be extended further than the '0'th frame of the underlying video provider.
        if (!mShiftDown)
        {
            lowerlimit(diff, getLeftEmptyArea(mOriginalClip));  // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            lowerlimit(diff, getLeftEmptyArea(linked));         // When not shift trimming: extended link must fit into the available empty area in front of the link
        }
    }
    else
    {
        lowerlimit(diff, mOriginalClip->getMinAdjustEnd());     // Clip cannot be trimmed further than the original number of frames
        lowerlimit(diff, linked->getMinAdjustEnd());            // Clip cannot be trimmed further than the original number of frames in the linked clip
        upperlimit(diff, mOriginalClip->getMaxAdjustEnd());     // Clip cannot be extended further than the last frame of the underlying video provider.
        upperlimit(diff, linked->getMaxAdjustEnd());            // Link cannot be extended further than the '0'th frame of the underlying video provider.
        if (!mShiftDown)
        {
            upperlimit(diff, getRightEmptyArea(mOriginalClip)); // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            upperlimit(diff, getRightEmptyArea(linked));        // When not shift trimming: extended link must fit into the available empty area in front of the link
        }

    }
    return diff;
}

void TrimBegin::preview()
{
    model::ClipPtr updatedClip = make_cloned<model::Clip>(mOriginalClip);
    // todo also show preview of adjacent clip
    if (updatedClip->isA<model::VideoClip>())
    {
        if (mTrimBegin)
        {
            updatedClip->adjustBegin(getDiff());
        }
        else
        {
            updatedClip->adjustEnd(getDiff());
        }

        if (updatedClip->getNumberOfFrames() > 0)
        { 
            model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(updatedClip);
            VAR_DEBUG(*mOriginalClip)(*updatedClip);
            if (mTrimBegin)
            {
                videoclip->moveTo(0);
            }
            else
            {
                videoclip->moveTo(videoclip->getNumberOfFrames() - 1);
            }
            wxSize s = mEdit->getSize();
            model::VideoFramePtr videoFrame = videoclip->getNextVideo(s.GetWidth(), s.GetHeight(), false);
            boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
            mEdit->show(bmp);
        }
    }
}

void TrimBegin::show()
{
    bool update = false;

    // Do not use mOriginalClip: it may have been removed from the track by applying command::Trim previously
    if (mMustUndo)
    {
        model::Project::current()->GetCommandProcessor()->Undo();
        mMustUndo = false;
        update = true;
    }
    // From here we can safely use mOriginalClip again

    pts diff = getDiff();
    if (diff != 0)
    {
        update = true;

        preview();

        model::Project::current()->Submit(new command::TrimBegin(getTimeline(), mOriginalClip, diff, mTrimBegin, mShiftDown));
        // From here we can no longer use mOriginalClip: it is changed by applying the command::TrimBegin

        mMustUndo = true;
        if (mShiftDown && mTrimBegin)
        {
            // Ensure that the rightmost pts is kept at the same position when shift dragging
            getScrolling().align(mFixedPts - diff, mFixedPixel);
        }
    }
    if (update)
    {
        getTimeline().Update();
    }
}

}}} // namespace