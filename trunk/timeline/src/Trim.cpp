// Copyright 2013 Eric Raijmakers.
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

#include "Trim.h"

#include "Clip.h"
#include "ClipView.h"
#include "Config.h"
#include "Convert.h"
#include "Cursor.h"
#include "Details.h"
#include "DetailsTrim.h"
#include "EmptyClip.h"
#include "Keyboard.h"
#include "Layout.h"
#include "Mouse.h"
#include "Player.h"
#include "PositionInfo.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Tooltip.h"
#include "Track.h"
#include "Transition.h"
#include "TrimClip.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilList.h"
#include "VideoClip.h"
#include "VideoFrame.h"
#include "VideoCompositionParameters.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Trim::Trim(Timeline* timeline)
    :   Part(timeline)
    ,   mActive(false)
    ,   mStartPosition(0,0)
    ,   mStartPts(0)
    ,   mFixedPixel(0)
    ,   mCommand(0)
    ,   mDc()
    ,   mSnappingEnabled(true)
{
    VAR_DEBUG(this);
}

Trim::~Trim()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRIM OPERATION
//////////////////////////////////////////////////////////////////////////

void Trim::start()
{
    LOG_DEBUG;

    // Reset first
    mActive = true;
    mSnappingEnabled = true;
    mStartPosition = getMouse().getPhysicalPosition(); // Do not replace with virtual position since the virtual canvas is changed because of shift trimming and keeping one clip edge aligned.
    mStartPts = 0;
    mFixedPixel = 0;
    mSnapPoints.clear();

    // Determine if pointer was at begin or at end of clip
    wxPoint virtualMousePosition = getMouse().getLeftDownPosition();
    PointerPositionInfo info = getMouse().getInfo(virtualMousePosition);
    ASSERT(info.clip && !info.clip->isA<model::EmptyClip>())(info);
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(info.clip);
    mPosition = info.logicalclipposition;

    model::IClipPtr mOriginalClip;
    model::IClipPtr adjacentClip;
    bool isBeginTrim = true;
    switch (mPosition)
    {
    case ClipBegin:
        ASSERT(!transition);
        ASSERT(!info.clip->isA<model::Transition>());
        mOriginalClip = info.clip;
        mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        mStartPts = mOriginalClip->getLeftPts();
        adjacentClip = mOriginalClip->getPrev();
        if (adjacentClip)
        {
            if (adjacentClip->getLength() == 0)
            {
                // Clip is part of a transition, and is 'fully covered' by this transition. Use the transition for the adjacent frame.
                adjacentClip = adjacentClip->getPrev();
                ASSERT(adjacentClip);
                ASSERT(adjacentClip->isA<model::Transition>())(adjacentClip);
            }
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
        break;
    case ClipEnd:
        ASSERT(!transition);
        ASSERT(!info.clip->isA<model::Transition>());
        mOriginalClip = info.clip;
        mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        mStartPts = mOriginalClip->getRightPts();
        adjacentClip = mOriginalClip->getNext();
        if (adjacentClip)
        {
            if (adjacentClip->getLength() == 0)
            {
                // Clip is part of a transition, and is 'fully covered' by this transition. Use the transition for the adjacent frame.
                adjacentClip = adjacentClip->getNext();
                ASSERT(adjacentClip);
                ASSERT(adjacentClip->isA<model::Transition>())(adjacentClip);
            }
            adjacentClip->moveTo(0);
        }
        isBeginTrim = false;
        break;
    case TransitionRightClipBegin:
        ASSERT(transition);
        ASSERT_MORE_THAN_ZERO(transition->getRight());
        mOriginalClip = info.clip->getNext(); // todo use info.getlogicalclip
        mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        mStartPts = mOriginalClip->getLeftPts();
        if (transition->getLeft() > 0)
        {
            adjacentClip = make_cloned<model::IClip>(transition->getPrev());
            ASSERT(adjacentClip);
            adjacentClip->adjustBegin(adjacentClip->getLength());
            adjacentClip->adjustEnd(transition->getLeft());
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
        break;
    case TransitionLeftClipEnd:
        ASSERT(transition);
        ASSERT_MORE_THAN_ZERO(transition->getLeft());
        mOriginalClip = info.clip->getPrev();
        mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        mStartPts = mOriginalClip->getRightPts();
        if (transition->getRight() > 0)
        {
            adjacentClip = make_cloned<model::IClip>(transition->getNext());
            ASSERT(adjacentClip);
            adjacentClip->adjustEnd(- adjacentClip->getLength());
            adjacentClip->adjustBegin(-transition->getRight());
            adjacentClip->moveTo(0);
        }
        isBeginTrim = false;
        break;
    case TransitionBegin:
        ASSERT(transition);
        mOriginalClip = info.clip;
        mStartPts = mOriginalClip->getLeftPts();
        transition.reset();
        break;
    case TransitionEnd:
        ASSERT(transition);
        mOriginalClip = info.clip;
        mStartPts = mOriginalClip->getRightPts();
        transition.reset();
        isBeginTrim = false;
        break;
    case ClipInterior:
    case TransitionLeftClipInterior:
    case TransitionInterior:
    case TransitionRightClipInterior:
    default:
        FATAL("Illegal clip position");
    }
    ASSERT(mOriginalClip);

    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.
    if (adjacentClip && (adjacentClip->isA<model::IVideo>()))
    {
        // This applies to both VideoClip and EmptyClip
        model::IVideoPtr adjacentvideoclip = boost::dynamic_pointer_cast<model::IVideo>(adjacentClip);
        model::VideoFramePtr adjacentFrame = adjacentvideoclip->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(getPlayer()->getVideoSize().GetWidth() / 2,  getPlayer()->getVideoSize().GetHeight())));
        mAdjacentBitmap = adjacentFrame->getBitmap();
    }

    // Prepare for previewing
    mPreviewVideoClip = boost::dynamic_pointer_cast<model::VideoClip>(make_cloned<model::IClip>(mOriginalClip));
    if (mPreviewVideoClip)
    {
        wxSize playerSize = getPlayer()->getVideoSize();

        model::TransitionPtr inTransition = mOriginalClip->getInTransition();
        model::TransitionPtr outTransition = mOriginalClip->getOutTransition();

        pts originalOffset = mPreviewVideoClip->getOffset();
        pts originalLength = mPreviewVideoClip->getLength();
        if (isBeginTrim)
        {
            mStartPositionPreview = originalOffset; // Left begin point
            if (inTransition)
            {
                mStartPositionPreview -= inTransition->getRight(); // Since the trim starts on the position where the two transitioned clips 'touched'
            }
        }
        else
        {
            mStartPositionPreview = originalOffset + originalLength - 1; // -1 required since end is 'one too far
            if (outTransition)
            {
                mStartPositionPreview += outTransition->getLeft(); // Since the trim starts on the position where the two transitioned clips 'touched'
            }
        }
        mPreviewVideoClip->maximize();

        mDc.SetBrush(Layout::get().PreviewBackgroundBrush);
        mDc.SetPen(Layout::get().PreviewBackgroundPen);
        mBitmapSingle = boost::make_shared<wxBitmap>(playerSize);
        mDc.SelectObject(*mBitmapSingle);
        mDc.DrawRectangle(wxPoint(0,0),mDc.GetSize());
        if (mAdjacentBitmap)
        {
            mBitmapSideBySide = boost::make_shared<wxBitmap>(playerSize);
            mDc.SelectObject(*mBitmapSideBySide);
            int xAdjacent = (isBeginTrim ? 0 : playerSize.GetWidth() / 2);
            mDc.DrawBitmap(*mAdjacentBitmap, xAdjacent, (playerSize.GetHeight() - mAdjacentBitmap->GetHeight()) / 2);
        }
        mDc.SelectObject(wxNullBitmap);
    }

    mCommand = new command::TrimClip(getSequence(), mOriginalClip, transition, mPosition);
    determinePossibleSnapPoints(mOriginalClip);
    update();
}

void Trim::update()
{
    VAR_DEBUG(this);
    getTimeline().beginTransaction();

    mCommand->update(determineTrimDiff(),false);
    getTimeline().getDetails().get<DetailsTrim>()->show( mCommand->getOriginalClip(), mCommand->getNewClip(), mCommand->getOriginalLink(), mCommand->getNewLink());
    preview();

    if (getKeyboard().getShiftDown() && mCommand->isBeginTrim())
    {
        // Ensure that the rightmost pts is kept at the same position when shift dragging
        getTimeline().setShift(getZoom().ptsToPixels(mCommand->getDiff()));
    }
    else
    {
        // This is required for the case in which a user shift-trims (timeline shift is changed),
        // and then releases shift. The release of shift implies shift must be reset.
        getTimeline().setShift(0);
    }

    getTimeline().endTransaction();
    getTimeline().Update();
}

void Trim::toggleSnapping()
{
    mSnappingEnabled = !mSnappingEnabled;
    update();
}

void Trim::stop()
{
    VAR_DEBUG(this);
    mActive = false;
    getTimeline().getDetails().get<DetailsTrim>()->hide();

    // Store before destroying mCommand
    model::IClipPtr originalclip = mCommand->getOriginalClip();
    model::IClipPtr originallink = mCommand->getOriginalLink();
    model::IClipPtr newclip = mCommand->getNewClip();
    model::IClipPtr newlink = mCommand->getNewLink();
    if (mCommand && !mCommand->isInitialized())
    {
        // The command has not yet been submitted. Undo all changes and feedback.
        delete mCommand; // Undo
        getTimeline().setShift(0);
        getTimeline().Refresh(false);
        getTimeline().Update();
    }
    mCommand = 0;

}

void Trim::submit()
{
    VAR_DEBUG(this);
    if (mCommand->getDiff() != 0)
    {
        bool shiftBeginTrim = getKeyboard().getShiftDown() && mCommand->isBeginTrim();
        pts diff = mCommand->getDiff();

        // Only submit the command if there's an actual diff to be applied
        // This call causes mCommand->isInitialized() to return true. See 'stop()'
        mCommand->submit();

        if (shiftBeginTrim)
        {
            // First, try changing the scrollbar such that the fixed pixel stays at the same position
            pixel remaining = getScrolling().align(mFixedPts,mFixedPixel + getZoom().ptsToPixels(diff));

            // If scrolling could completely align the pts value with the given pixel (typically happens
            // when trimming at the begin of the timeline), show an animation of the sequence moving to the
            // beginning of the timeline.
            if (remaining < 0)
            {
                static const int SleepTimePerStep = 25;
                static const int AnimationDurationInMs = 250;
                static const int NumberOfSteps = AnimationDurationInMs / SleepTimePerStep;
                for (int step = NumberOfSteps; step >= 0; --step)
                {
                    int newShift = -1 * model::Convert::doubleToInt(static_cast<double>(remaining) / static_cast<double>(NumberOfSteps) * static_cast<double>(step));
                    getTimeline().setShift(newShift);
                    getTimeline().Refresh(false);
                    getTimeline().Update();
                    boost::this_thread::sleep(boost::posix_time::milliseconds(SleepTimePerStep));
                }
            }
            else
            {
                getTimeline().setShift(0);
                getTimeline().Refresh(false);
                getTimeline().Update();
            }
        }
    }
}

void Trim::draw(wxDC& dc) const
{
    if (!mActive)
    {
        return;
    }
    dc.SetPen(Layout::get().SnapPen);
    dc.SetBrush(Layout::get().SnapBrush);

    if (mSnap)
    {
        pixel pos = getZoom().ptsToPixels(*mSnap) - getTimeline().getShift();
        dc.DrawLine(pos,0,pos,dc.GetSize().GetHeight());
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pts Trim::determineTrimDiff()
{
    wxPoint position = getMouse().getPhysicalPosition(); // Do not replace with virtual position since the virtual canvas is changed because of shift trimming and keeping one clip edge aligned.
    pts result = getZoom().pixelsToPts(position.x - mStartPosition.x);
    if (mSnap)
    {
        getTimeline().refreshPts(*mSnap);
        mSnap.reset();
    }
    if (mSnappingEnabled)
    {
        pts ptsmouse = getZoom().pixelsToPts(position.x + getScrolling().getOffset().x);
        pts minimumsnap = Layout::SnapDistance + 1; // To ensure that the first snap will update this
        std::list<pts>::const_iterator itTimeline = mSnapPoints.begin();
        while ( itTimeline != mSnapPoints.end() )
        {
            pts pts_timeline = *itTimeline;
            pts diff = ptsmouse - pts_timeline;
            if ((abs(diff) <= Layout::SnapDistance) && (abs(diff) < abs(minimumsnap)))
            {
                minimumsnap = diff;
                result = pts_timeline - mStartPts;
                mSnap.reset(pts_timeline);
            }
            ++itTimeline;
        }
        if (mSnap)
        {
            getTimeline().refreshPts(*mSnap);
        }
    }
    return result;
}

void Trim::determinePossibleSnapPoints(model::IClipPtr originalclip)
{
    pts min = 0; // Use these to limit the number of snap points (performance)
    pts max = 0;
    if (mCommand->isBeginTrim())
    {
        min = originalclip->getLeftPts() + originalclip->getMinAdjustBegin();
        max = originalclip->getLeftPts() + originalclip->getMaxAdjustBegin();
    }
    else
    {
        min = originalclip->getRightPts() + originalclip->getMinAdjustEnd();
        max = originalclip->getRightPts() + originalclip->getMaxAdjustEnd();
    }
    mSnapPoints.clear();
    if (Config::ReadBool(Config::sPathSnapClips))
    {
        std::set<model::IClipPtr> exclude = boost::assign::list_of(originalclip)(originalclip->getLink());
        std::list<pts> all;
        UtilList<pts>(all).addElements(getSequence()->getCuts(exclude));
        // Copy everything between [min,max), discard everything else
        mSnapPoints.splice(mSnapPoints.begin(),all,std::lower_bound(all.begin(),all.end(), min), std::upper_bound(all.begin(),all.end(),max));
    }
    if (Config::ReadBool(Config::sPathSnapCursor))
    {
        mSnapPoints.push_back(getCursor().getLogicalPosition());
    }
    mSnapPoints.sort();
    mSnapPoints.unique();
    VAR_DEBUG(originalclip)(min)(max)(mSnapPoints);
}

void Trim::preview()
{
    if (!mPreviewVideoClip) { return; }

    pts diff = mCommand->getDiff();
    pts position(mStartPositionPreview + diff);
    wxSize playerSize = getPlayer()->getVideoSize();
    bool isBeginTrim = mCommand->isBeginTrim();
    bool drawSideBySide = getKeyboard().getShiftDown() && mAdjacentBitmap;

    bool completelyTrimmedAway = false;
    ASSERT_LESS_THAN_EQUALS(mStartPositionPreview + diff,mPreviewVideoClip->getLength());
    if (position == mPreviewVideoClip->getLength())
    {
        ASSERT(isBeginTrim);
        completelyTrimmedAway = true; // Clip has been trimmed away completely. From the beginning.
    }
    ASSERT_MORE_THAN_EQUALS(position,-1);
    if (position == -1)
    {
        ASSERT(!isBeginTrim);
        completelyTrimmedAway = true; // Clip has been trimmed away completely. From the end.
    }

    pixel w = playerSize.GetWidth();
    pixel h = playerSize.GetHeight();
    wxBitmapPtr bitmap = mBitmapSingle;
    if (drawSideBySide)
    {
        w = w / 2;
        bitmap = mBitmapSideBySide;
    }
    int x = isBeginTrim ? playerSize.GetWidth() - w : 0;

    mDc.SelectObject(*bitmap);
    if (completelyTrimmedAway)
    {
        mDc.SetBrush(Layout::get().PreviewBackgroundBrush);
        mDc.SetPen(Layout::get().PreviewBackgroundPen);
        mDc.DrawRectangle(x, 0, w, h);
    }
    else
    {
        mPreviewVideoClip->moveTo(position);
        model::VideoFramePtr videoFrame = mPreviewVideoClip->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(w, h)));
        ASSERT(videoFrame); // A frame must be possible, due to the 'completelyTrimmedAway' check above.
        wxBitmapPtr trimmedBmp = videoFrame->getBitmap();
        ASSERT(trimmedBmp);
        mDc.DrawBitmap(*trimmedBmp, x, (h - trimmedBmp->GetHeight()) / 2);
    }
    mDc.SelectObject(wxNullBitmap);
    getPlayer()->show(bitmap);
}

}} // namespace