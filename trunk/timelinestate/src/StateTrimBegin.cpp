#include "StateTrimBegin.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/cmdproc.h>
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
    ,   mMinDiffClipContent(0)
    ,   mMaxDiffClipContent(0)
    ,   mMinDiffClipSpace(0)
    ,   mMinDiffLinkContent((std::numeric_limits<pts>::min)())
    ,   mMaxDiffLinkContent((std::numeric_limits<pts>::max)())
    ,   mMinDiffLinkSpace((std::numeric_limits<pts>::min)())
    ,   mMinShiftOtherTrackContent((std::numeric_limits<pts>::min)())
    ,   mMustUndo(false)
    ,   mShiftDown(false)
{
    LOG_DEBUG; 

    const EvLeftDown* event = dynamic_cast<const EvLeftDown*>(triggering_event());
    mShiftDown = event->mWxEvent.ShiftDown();
    mOriginalPointerPosition = getZoom().pixelsToPts(event->mPosition.x);
    ASSERT(event); // Only way to get here is to press left button in the Idle state

    // \todo use the leftmost of the clip and/or its link
    mStartPosition = event->mPosition;
    mCurrentPosition = mStartPosition;
    PointerPositionInfo info = getMousePointer().getInfo(mCurrentPosition);
    mOriginalClip = info.clip;

    // Determine boundaries for original clip
    mMaxDiffClipContent = mOriginalClip->getNumberOfFrames() - 1; // -1: Ensure that resulting clip has always minimally one frame left
    mMinDiffClipContent = -mOriginalClip->getOffset() + 1; // +1: Ensure that resulting clip has always minimally one frame left 
    mMinDiffClipSpace = getLeftEmptyArea(mOriginalClip);

    // Determine boundaries for linked clip
    model::ClipPtr linked = mOriginalClip->getLink();
    if (linked)
    {
        mMaxDiffLinkContent = linked->getNumberOfFrames() - 1; // -1: Ensure that resulting clip has always minimally one frame left
        mMinDiffLinkContent = -linked->getOffset() + 1; // +1: Ensure that resulting clip has always minimally one frame left 
        mMinDiffLinkSpace = getLeftEmptyArea(linked);
    }

    // Determine boundaries for shifting other tracks
    pts shiftFrom = mOriginalClip->getLeftPts();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        if (mOriginalClip->getTrack() == track) continue;
        if (linked && linked->getTrack() == track) continue;
        model::ClipPtr clipAt = track->getClip(shiftFrom);
        mMinShiftOtherTrackContent = 
            (clipAt->isA<model::EmptyClip>()) ? std::max<pts>(mMinShiftOtherTrackContent, clipAt->getLeftPts() - shiftFrom) : 0;
    }
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        if (mOriginalClip->getTrack() == track) continue;
        if (linked && linked->getTrack() == track) continue;
        model::ClipPtr clipAt = track->getClip(shiftFrom);
        mMinShiftOtherTrackContent = 
            (clipAt->isA<model::EmptyClip>()) ? std::max<pts>(mMinShiftOtherTrackContent, clipAt->getLeftPts() - shiftFrom) : 0;
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
    if (mCurrentPosition != evt.mPosition)
    {
        mCurrentPosition = evt.mPosition;
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

pts TrimBegin::getDiff()
{
    return
        std::min<pts>(mMaxDiffClipContent,
        std::max<pts>(mShiftDown ? mMinDiffClipContent : mMinDiffClipSpace,
        std::max<pts>(mMinDiffClipContent,
        std::max<pts>(mShiftDown ? mMinShiftOtherTrackContent : (std::numeric_limits<pts>::min)(),
        getZoom().pixelsToPts(mCurrentPosition.x - mStartPosition.x)))));
}

model::ClipPtr TrimBegin::getUpdatedClip()
{
    pts diff = getDiff();
    if (diff == 0) return model::ClipPtr();
    model::ClipPtr clip = make_cloned<model::Clip>(mOriginalClip);
    clip->adjustBegin(getDiff());
    return clip;
}
 
void TrimBegin::show()
{
    if (mMustUndo)
    {
        model::Project::current()->GetCommandProcessor()->Undo();
        mMustUndo = false;
    }

    model::ClipPtr updatedClip = getUpdatedClip();
    if (updatedClip)
    {
        if (updatedClip->isA<model::VideoClip>())
        { 
            model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(updatedClip);
            VAR_DEBUG(*mOriginalClip)(*updatedClip);
            videoclip->moveTo(0);
            //VAR_DEBUG(*mOriginalClip)(*updatedClip);
            wxSize s = mEdit->getSize();
            model::VideoFramePtr videoFrame = videoclip->getNextVideo(s.GetWidth(), s.GetHeight(), false);
            boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
            mEdit->show(bmp);
        }
        pts beginPos = mOriginalClip->getLeftPts() + getDiff(); // Must be initialized before executing the command (since that'll remove mOriginalClip from the track)

        model::Project::current()->Submit(new command::TrimBegin(getTimeline(), mOriginalClip, getDiff(), mShiftDown));
        mMustUndo = true;
        if (mShiftDown)
        {
            getScrolling().align(mOriginalPointerPosition, getZoom().pixelsToPts(mCurrentPosition.x));
        }
    }
}

}}} // namespace