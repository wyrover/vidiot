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
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilVector.h"
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
    mCursorPositionBefore = getCursor().getLogicalPosition();

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
        {
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
        }
    case ClipEnd:
        {
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
        }
    case TransitionRightClipBegin:
        {
            ASSERT(transition);
            ASSERT(transition->getRight());
            ASSERT_MORE_THAN_EQUALS_ZERO(*(transition->getRight()));
            mStartPts = transition->getTouchPosition();
            mOriginalClip = info.getLogicalClip();
            mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
            boost::optional<pts> left = transition->getLeft();
            if (left)
            {
                adjacentClip = make_cloned<model::IClip>(transition->getPrev());
                if (*left > 0)
                {
                    ASSERT(adjacentClip);
                    adjacentClip->adjustBegin(adjacentClip->getLength());
                    adjacentClip->adjustEnd(*left);
                }
                adjacentClip->moveTo(adjacentClip->getLength() - 1);
            }
            break;
        }
    case TransitionLeftClipEnd:
        {
            ASSERT(transition);
            ASSERT(transition->getLeft());
            ASSERT_MORE_THAN_EQUALS_ZERO(*(transition->getLeft()));
            mStartPts = transition->getTouchPosition();
            mOriginalClip = info.getLogicalClip();
            mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
            boost::optional<pts> right = transition->getRight();
            if (right)
            {

                adjacentClip = make_cloned<model::IClip>(transition->getNext());
                if (*right > 0)
                {
                    ASSERT(adjacentClip);
                    adjacentClip->adjustEnd(- adjacentClip->getLength());
                    adjacentClip->adjustBegin(-1 * *right);
                }
                adjacentClip->moveTo(0);
            }
            isBeginTrim = false;
            break;
        }
    case TransitionBegin:
        {
            ASSERT(transition);
            mOriginalClip = info.clip;
            mStartPts = mOriginalClip->getLeftPts();
            transition.reset();
            break;
        }
    case TransitionEnd:
        {
            ASSERT(transition);
            mOriginalClip = info.clip;
            mStartPts = mOriginalClip->getRightPts();
            transition.reset();
            isBeginTrim = false;
            break;
        }
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
            if (inTransition && inTransition->getRight())
            {
                mStartPositionPreview -= *(inTransition->getRight()); // Since the trim starts on the position where the two transitioned clips 'touched'
            }
        }
        else
        {
            mStartPositionPreview = originalOffset + originalLength - 1; // -1 required since end is 'one too far
            if (outTransition && outTransition->getLeft())
            {
                mStartPositionPreview += *(outTransition->getLeft()); // Since the trim starts on the position where the two transitioned clips 'touched'
            }
        }
        mPreviewVideoClip->maximize();

        mBitmapSingle = boost::make_shared<wxBitmap>(playerSize);
        mDc.SelectObject(*mBitmapSingle);
        mDc.SetBrush(Layout::get().PreviewBackgroundBrush);
        mDc.SetPen(Layout::get().PreviewBackgroundPen);
        mDc.DrawRectangle(wxPoint(0,0),mDc.GetSize());
        if (mAdjacentBitmap)
        {
            mBitmapSideBySide = boost::make_shared<wxBitmap>(playerSize);
            mDc.SelectObject(*mBitmapSideBySide);
            int xAdjacent = (isBeginTrim ? 0 : playerSize.GetWidth() / 2);

            wxMemoryDC dcBmp(*mAdjacentBitmap);
            mDc.Blit(wxPoint(xAdjacent, (playerSize.GetHeight() - mAdjacentBitmap->GetHeight()) / 2),mAdjacentBitmap->GetSize(),&dcBmp,wxPoint(0,0));
        }
        mDc.SelectObject(wxNullBitmap);
    }

	ASSERT_ZERO(mCommand);
    getSelection().updateOnTrim(mOriginalClip);
    mCommand = new command::TrimClip(getSequence(), mOriginalClip, transition, mPosition);
    determinePossibleSnapPoints(mOriginalClip);
    // Fix the length of the timeline such that it will never be shortened during
    // the trim operation. Otherwise, trimming at the end of the sequence gives
    // awkward feedback due to the shrinking of the widget.
    getSequenceView().setMinimumLength(getSequenceView().getDefaultLength()); 
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
    getTimeline().Refresh();
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
    getSequenceView().setMinimumLength(0);
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
    mActive = false; // Ensure snaps are no longer shown (typical case: shfit begin trim with snap to cursor)
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

            // When the sequence before the cursor position is 'shortened', without adaptation, the cursor
            // seems to be moved after the trim is done. Although technically, the cursor is kept in the
            // same position (absolute value) it ends up on a different logical place (since the
            // sequence has become shorter). Move the cursor, to ensure that it remains in the same
            // position logically.
            //
            // For this reason, just before updating the timeline the cursor is repositioned such that
            // it 'seems' to stick in the same place. This is done both during the animation AND at the
            // end, for cases without animation.

            // If scrolling could NOT completely align the pts value with the given pixel (typically happens
            // when trimming at the begin of the timeline), show an animation of the sequence moving to the
            // beginning of the timeline.

            if (remaining < 0)
            {
                static const int SleepTimePerStep = 25;
                static const int AnimationDurationInMs = 250;
                static const int NumberOfSteps = AnimationDurationInMs / SleepTimePerStep;
                for (int step = NumberOfSteps; step > 0; --step) // step > 0: otherwise /0 possible
                {
                    int newShift = -1 * model::Convert::doubleToInt(static_cast<double>(remaining) / static_cast<double>(NumberOfSteps) * static_cast<double>(step));
                    pts cursorDiff = getTimeline().getZoom().pixelsToPts(newShift);
                    getTimeline().setShift(newShift);
                    getTimeline().getCursor().setLogicalPosition(mCursorPositionBefore - diff + cursorDiff);
                    getTimeline().Update();
                    boost::this_thread::sleep(boost::posix_time::milliseconds(SleepTimePerStep));
                }
            }
            getTimeline().setShift(0);
            getTimeline().getCursor().setLogicalPosition(mCursorPositionBefore - diff);
            getTimeline().Update();
        }
    }
}

void Trim::drawSnaps(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (mActive && mSnap)
    {
        getTimeline().drawLine(dc,region,offset,*mSnap,Layout::get().SnapPen);
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
        std::vector<pts>::const_iterator itTimeline = mSnapPoints.begin();
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

void Trim::determinePossibleSnapPoints(const model::IClipPtr& originalclip)
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
        std::vector<pts> all;
        UtilVector<pts>(all).addElements(getSequence()->getCuts(exclude));
        // Copy everything between [min,max), discard everything else
        mSnapPoints.insert(mSnapPoints.begin(),std::lower_bound(all.begin(),all.end(), min), std::upper_bound(all.begin(),all.end(),max));
    }
    if (Config::ReadBool(Config::sPathSnapCursor))
    {
        mSnapPoints.push_back(getCursor().getLogicalPosition());
    }
    std::sort(mSnapPoints.begin(), mSnapPoints.end());
    std::unique(mSnapPoints.begin(), mSnapPoints.end());
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
        if (trimmedBmp)
        {
            // todo GCC first edit operation, the editdisplay does not show bitmap until first 'idle' moment?
            wxMemoryDC dcBmp(*trimmedBmp);
            mDc.Blit(wxPoint(x, (h - trimmedBmp->GetHeight()) / 2), trimmedBmp->GetSize(), &dcBmp, wxPoint(0,0));
        }
    }
    mDc.SelectObject(wxNullBitmap);
    getPlayer()->show(bitmap);
}

}} // namespace
