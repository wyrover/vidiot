#include "StateTrim.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/cmdproc.h>
#include <wx/dcmemory.h>
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
#include "Player.h"
#include "EditDisplay.h"
#include "Sequence.h"
#include "Scrolling.h"
#include "EmptyClip.h"
#include "Track.h"
#include "VideoClip.h"
#include "ClipView.h"
#include "Layout.h"
#include "Project.h"
#include "Zoom.h"
#include "Timeline.h"
#include "Trim.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _(
    "Move the cursor to change the entry point of the clip.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Trim::Trim( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mStartPosition(0,0)
    ,   mEdit(getPlayer()->startEdit())
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
    model::IClipPtr adjacentClip;
    if (mTrimBegin)
    {
        mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clip's are added/removed, that's very volatile information).
        adjacentClip = mOriginalClip->getTrack()->getPreviousClip(mOriginalClip);
        if (adjacentClip)
        {
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
    }
    else
    {
        mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clip's are added/removed, that's very volatile information).
        adjacentClip = mOriginalClip->getTrack()->getNextClip(mOriginalClip);
        if (adjacentClip)
        {
            adjacentClip->moveTo(0);
        }
    }
    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.
    if (adjacentClip && adjacentClip->isA<model::VideoClip>())
    {
        model::VideoClipPtr adjacentvideoclip = boost::dynamic_pointer_cast<model::VideoClip>(adjacentClip);
        model::VideoFramePtr adjacentFrame = adjacentvideoclip->getNextVideo(mEdit->getSize().GetWidth() / 2,  mEdit->getSize().GetHeight(), false);
        mAdjacentBitmap = boost::make_shared<wxBitmap>(wxImage(adjacentFrame->getWidth(), adjacentFrame->getHeight(), adjacentFrame->getData()[0], true));
    }

    // Determine boundaries for shifting other tracks
    // TODO more testing
    model::IClipPtr linked = mOriginalClip->getLink();
    pts shiftFrom = mOriginalClip->getLeftPts();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        if (mOriginalClip->getTrack() == track) continue;
        if (linked && linked->getTrack() == track) continue;
        model::IClipPtr clipAt = track->getClip(shiftFrom);
        mMinShiftOtherTrackContent = 
            (clipAt->isA<model::EmptyClip>()) ? std::max<pts>(mMinShiftOtherTrackContent, clipAt->getLeftPts() - shiftFrom) : 0;
        mMaxShiftOtherTrackContent = 
            (clipAt->isA<model::EmptyClip>()) ? std::min<pts>(mMaxShiftOtherTrackContent, clipAt->getRightPts() - shiftFrom) : 0;
    }

    preview();
}

Trim::~Trim() // exit
{
    getPlayer()->endEdit();
    LOG_DEBUG; 
}
//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Trim::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result Trim::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    if (mCurrentPosition != evt.mWxEvent.GetPosition())
    {
        mCurrentPosition = evt.mWxEvent.GetPosition();
        show();
    }
    return forward_event();
}

boost::statechart::result Trim::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    if (undo())
    {
        getTimeline().Update();
    }
    return transit<Idle>();
}

boost::statechart::result Trim::react( const EvKeyDown& evt)
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
    case WXK_ESCAPE:
        if (undo())
        {
            getTimeline().Update();
        }
        return transit<Idle>();
    }
    return forward_event();
}

boost::statechart::result Trim::react( const EvKeyUp& evt)
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

void lowerlimit(pts& p, pts limit)
{
    if (p < limit) { p = limit; }
}

void upperlimit(pts& p, pts limit)
{
    if (p > limit) { p = limit; }
}

pts Trim::getDiff()
{
    ASSERT(!mMustUndo); // If a command has been submitted, mOriginalClip can not be used.

    pts diff = getZoom().pixelsToPts(mCurrentPosition.x - mStartPosition.x);
    model::IClipPtr linked = mOriginalClip->getLink();

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
            lowerlimit(diff, mOriginalClip->getTrack()->getLeftEmptyArea(mOriginalClip));   // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            lowerlimit(diff, linked->getTrack()->getLeftEmptyArea(linked));                 // When not shift trimming: extended link must fit into the available empty area in front of the link
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
            upperlimit(diff, mOriginalClip->getTrack()->getRightEmptyArea(mOriginalClip));  // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            upperlimit(diff, linked->getTrack()->getRightEmptyArea(linked));                // When not shift trimming: extended link must fit into the available empty area in front of the link
        }
    }
    return diff;
}

void Trim::preview()
{
    model::IClipPtr updatedClip = make_cloned<model::IClip>(mOriginalClip);
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

        if (updatedClip->getLength() > 0)
        { 
            model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(updatedClip);
            wxSize s = mEdit->getSize();
            bool drawadjacentclip = mShiftDown && mAdjacentBitmap;
            int previewwidth = (drawadjacentclip ? s.GetWidth() / 2 : s.GetWidth());
            int previewxpos = 0;
            boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
            wxMemoryDC dc(*bmp);

            if (mTrimBegin)
            {
                videoclip->moveTo(0);
                previewxpos = s.GetWidth() - previewwidth; // This works for both with and without an adjacent clip
            }
            else
            {
                videoclip->moveTo(videoclip->getLength() - 1);
                previewxpos = 0;
            }

            // Fill with black
            dc.SetBrush(Layout::sPreviewBackgroundBrush);
            dc.SetPen(Layout::sPreviewBackgroundPen);
            dc.DrawRectangle(wxPoint(0,0),dc.GetSize());

            // Draw preview of trim operation
            model::VideoFramePtr videoFrame = videoclip->getNextVideo(previewwidth, s.GetHeight(), false);
            wxBitmap trimmedBmp = wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true));
            dc.DrawBitmap(trimmedBmp, previewxpos, (s.GetHeight() - trimmedBmp.GetHeight()) / 2);

            // Draw adjacent clip if present. Is only relevant when holding shift
            if (drawadjacentclip)
            {
                int xAdjacent = (mTrimBegin ? 0 : s.GetWidth() / 2);
                dc.DrawBitmap(*mAdjacentBitmap, xAdjacent, (s.GetHeight() - mAdjacentBitmap->GetHeight()) / 2);
            }
            dc.SelectObject(wxNullBitmap);
            mEdit->show(bmp);
        }
    }
}

void Trim::show()
{
    // Do not use mOriginalClip: it may have been removed from the track by applying command::Trim previously
    bool update = undo();
    // From here we can safely use mOriginalClip again

    pts diff = getDiff();
    if (diff != 0)
    {
        update = true;

        preview();

        model::Project::get().Submit(new command::Trim(getTimeline(), mOriginalClip, diff, mTrimBegin, mShiftDown));
        // From here we can no longer use mOriginalClip: it is changed by applying the command::Trim

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

bool Trim::undo()
{
    if (mMustUndo)
    {
        model::Project::get().GetCommandProcessor()->Undo();
        mMustUndo = false;
        return true;
    }
    return false;
}

}}} // namespace