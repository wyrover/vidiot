// Copyright 2013-2016 Eric Raijmakers.
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
#include "Mouse.h"
#include "Player.h"
#include "PositionInfo.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "TrimClip.h"
#include "UtilVector.h"
#include "VideoClip.h"
#include "VideoFrame.h"
#include "VideoTrack.h"
#include "VideoCompositionParameters.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Trim::Trim(Timeline* timeline)
    : Part(timeline)
    , mActive(false)
    , mStartPosition(0, 0)
    , mStartPts(0)
    , mFixedPixel(0)
    , mCommand(0)
    , mDc()
    , mSnappingEnabled(true)
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
    mTrimLink = true;
    // Do not replace with virtual position since the virtual canvas is
    // changed because of shift trimming and keeping one clip edge aligned.
    mStartPosition = getMouse().getPhysicalPosition();
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

    bool isBeginTrim = (mPosition == ClipBegin) || (mPosition == TransitionRightClipBegin) || (mPosition == TransitionBegin);

    boost::optional<pts> adjacentPts{ boost::none };

    model::IClipPtr originalclip;

    // Below, do not optimize away - using ->getRightPts() and/or ->getLeftPts() - in the calculation.
    // Since the scrolling is changed and clips are added/removed, that's very volatile information.
    switch (mPosition)
    {
        case ClipBegin:
        {
            ASSERT(!transition);
            ASSERT(!info.clip->isA<model::Transition>());
            originalclip = info.clip;
            mFixedPts = originalclip->getRightPts();
            mStartPts = originalclip->getLeftPts();
            adjacentPts.reset(mStartPts - 1);
            break;
        }
        case ClipEnd:
        {
            ASSERT(!transition);
            ASSERT(!info.clip->isA<model::Transition>());
            originalclip = info.clip;
            mFixedPts = originalclip->getLeftPts();
            mStartPts = originalclip->getRightPts();
            adjacentPts.reset(mStartPts + 1);
            break;
        }
        case TransitionRightClipBegin:
        {
            ASSERT(transition);
            ASSERT(transition->getRight());
            ASSERT_MORE_THAN_EQUALS_ZERO(*(transition->getRight()));
            mStartPts = transition->getTouchPosition();
            originalclip = info.getLogicalClip();
            mFixedPts = originalclip->getRightPts();
            if (!transition->getLeft() ||
                *transition->getLeft() == 0)
            {
                // In-only transition, or in-out transition with a left size of 0 length (total transition to the right of the cut).
                adjacentPts.reset(mStartPts - 1);
            }
            break;
        }
        case TransitionLeftClipEnd:
        {
            ASSERT(transition);
            ASSERT(transition->getLeft());
            ASSERT_MORE_THAN_EQUALS_ZERO(*(transition->getLeft()));
            mStartPts = transition->getTouchPosition();
            originalclip = info.getLogicalClip();
            mFixedPts = originalclip->getLeftPts();
            if (!transition->getRight() ||
                *transition->getRight() == 0)
            {
                // In-only transition, or in-out transition with right size of 0 length (total transition to the left of the cut).
                adjacentPts.reset(mStartPts + 1);
            }
            break;
        }
        case TransitionBegin:
        {
            ASSERT(transition);
            originalclip = info.clip;
            mStartPts = originalclip->getLeftPts();
            transition.reset();
            break;
        }
        case TransitionEnd:
        {
            ASSERT(transition);
            originalclip = info.clip;
            mStartPts = originalclip->getRightPts();
            transition.reset();
            break;
        }
        case ClipInterior:
        case TransitionLeftClipInterior:
        case TransitionInterior:
        case TransitionRightClipInterior:
        default:
        FATAL("Illegal clip position");
    }
    ASSERT(originalclip);

    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.

    // Determine the video clip to be used for previewing.
    mAdjacentBitmap.reset(); // Do not use bitmap stored for any previous trim operation
    mVideoClip.reset(); // Default: no video preview

    mVideoClip = boost::dynamic_pointer_cast<model::VideoClip>(originalclip);
    if (mVideoClip == nullptr)
    {
        // Maybe the trim was started on the audio. Use the linked clip for the preview.
        mVideoClip = boost::dynamic_pointer_cast<model::VideoClip>(originalclip->getLink());
    }

    if (mVideoClip)
    {
        // Determine (preview) pts offset position of trimmed clip. -1: required since end is 'one too far'.
        mStartPositionPreview = isBeginTrim ? mVideoClip->getOffset() : mVideoClip->getOffset() + mVideoClip->getLength() - 1;

        // For the first and last frames in the track, there are no adjacent bitmaps
        if (mStartPts > 0 &&
            mStartPts < mVideoClip->getTrack()->getLength() - 1)
        {
            model::IVideoPtr adjacentBitmapProvider;

            model::IClipPtr adjacentVideoClip{ isBeginTrim ? mVideoClip->getPrev() : mVideoClip->getNext() };
            model::TransitionPtr adjacentVideoTransition{ boost::dynamic_pointer_cast<model::Transition>(adjacentVideoClip) };

            model::VideoCompositionParameters parameters{ model::VideoCompositionParameters().
                setBoundingBox(wxSize(getPlayer()->getVideoSize().GetWidth() / 2, getPlayer()->getVideoSize().GetHeight())) };

            if (adjacentVideoTransition &&
                adjacentVideoTransition->getLeft() &&
                *adjacentVideoTransition->getLeft() > 0 &&
                adjacentVideoTransition->getRight() &&
                *adjacentVideoTransition->getRight() > 0)
            {
                // Special case: in case of a in-out transition, do not show the track's frame (which
                // is a combination of both the trimmed clip and it's adjacent clip. Instead show the
                // actual frames (for both the trimmed and the adjacent clip) around the 'touch' 
                // position.
                if (isBeginTrim)
                {
                    adjacentVideoClip = make_cloned<model::IClip>(adjacentVideoTransition->getPrev());
                    ASSERT(adjacentVideoClip);
                    adjacentVideoClip->adjustBegin(adjacentVideoClip->getLength()); // Remove 'not-under-the-transition' part.
                    adjacentVideoClip->adjustEnd(*adjacentVideoTransition->getLeft()); // Remove 'before-the-touch-position'.
                    adjacentVideoClip->moveTo(adjacentVideoClip->getLength() - 1); // Take clip besides 'touch' position.
                    mStartPositionPreview -= *adjacentVideoTransition->getRight(); // Updated offset for (preview) of trimmed clip.
                }
                else
                {
                    adjacentVideoClip = make_cloned<model::IClip>(adjacentVideoTransition->getNext());
                    ASSERT(adjacentVideoClip);
                    adjacentVideoClip->adjustEnd(-adjacentVideoClip->getLength()); // Remove 'not-under-the-transition' part.
                    adjacentVideoClip->adjustBegin(-1 * *adjacentVideoTransition->getRight()); // Remove 'after-the-touch-position' part.
                    adjacentVideoClip->moveTo(0); // Take clip besides 'touch' position.
                    mStartPositionPreview += *(adjacentVideoTransition->getLeft()); // Updated offset for (preview) of trimmed clip.
                }

                adjacentBitmapProvider = boost::dynamic_pointer_cast<model::IVideo>(adjacentVideoClip); // Works for VideoClip and EmptyClip
            }
            else
            {
                // Default: get the next/previous video frame in the track
                model::TrackPtr track{ mVideoClip->getTrack() };
                model::VideoTrackPtr videotrack{ make_cloned<model::VideoTrack>(boost::dynamic_pointer_cast<model::VideoTrack>(track)) };
                videotrack->moveTo(mStartPts + (isBeginTrim ? -1 : +1));
                adjacentBitmapProvider = boost::dynamic_pointer_cast<model::IVideo>(videotrack);
            }

            ASSERT(adjacentBitmapProvider != nullptr);
            model::VideoFramePtr adjacentFrame{ adjacentBitmapProvider->getNextVideo(parameters) };
            ASSERT(adjacentFrame != nullptr);
            mAdjacentBitmap = adjacentFrame->getBitmap();
        }
    }

    // Create preview bitmaps.
    wxSize playerSize = getPlayer()->getVideoSize();
    if (playerSize.x < 20) { playerSize.x = 20; }
    if (playerSize.y < 20) { playerSize.y = 20; }
    mBitmapSingle = boost::make_shared<wxBitmap>(playerSize);
    mDc.SelectObject(*mBitmapSingle);
    mDc.SetBrush(wxBrush{ wxColour{ 0, 0, 0 } });
    mDc.SetPen(wxPen{ wxColour{ 0, 0, 0 } });
    mDc.DrawRectangle(wxPoint(0, 0), mDc.GetSize());
    if (mAdjacentBitmap)
    {
        mBitmapSideBySide = boost::make_shared<wxBitmap>(playerSize);
        mDc.SelectObject(*mBitmapSideBySide);
        int xAdjacent = (isBeginTrim ? 0 : playerSize.GetWidth() / 2);

        wxMemoryDC dcBmp(*mAdjacentBitmap);
        mDc.Blit(wxPoint(xAdjacent, (playerSize.GetHeight() - mAdjacentBitmap->GetHeight()) / 2), mAdjacentBitmap->GetSize(), &dcBmp, wxPoint(0, 0));
    }
    mDc.SelectObject(wxNullBitmap);

    ASSERT_ZERO(mCommand);
    getSelection().updateOnTrim(originalclip);
    mCommand = new cmd::TrimClip(getSequence(), originalclip, transition, mPosition);
    determinePossibleSnapPoints(originalclip);
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

    mCommand->update(determineTrimDiff(), false, mTrimLink);
    getTimeline().getDetails().get<DetailsTrim>()->show(mCommand->getOriginalClip(), mCommand->getNewClip(), mCommand->getOriginalLink(), mCommand->getNewLink());
    preview();

    if (mCommand->isShiftTrim() && mCommand->isBeginTrim())
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
    getTimeline().Refresh(false);
    getTimeline().Update();
}

void Trim::toggleSnapping()
{
    mSnappingEnabled = !mSnappingEnabled;
    update();
}

void Trim::toggleTrimLink()
{
    mTrimLink = !mTrimLink;
    update();
}

void Trim::stop()
{
    VAR_DEBUG(this);
    mActive = false;
    getSequenceView().setMinimumLength(0);
    getDetails().get<DetailsTrim>()->hide();

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
    getPlayer()->showPlayer();
    mVideoClip.reset();
    mCommand = 0;
}

void Trim::submit()
{
    VAR_DEBUG(this);
    mActive = false; // Ensure snaps are no longer shown (typical case: shfit begin trim with snap to cursor)
    if (mCommand->getDiff() != 0)
    {
        bool shiftBeginTrim = mCommand->isShiftTrim() && mCommand->isBeginTrim();
        pts diff = mCommand->getDiff();

        // Only submit the command if there's an actual diff to be applied
        // This call causes mCommand->isInitialized() to return true. See 'stop()'
        mCommand->submit();

        if (shiftBeginTrim)
        {
            // First, try changing the scrollbar such that the fixed pixel stays at the same position
            pixel remaining = getScrolling().align(mFixedPts, mFixedPixel + getZoom().ptsToPixels(diff));

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
                    int newShift{ -1 * static_cast<int>(std::round(static_cast<double>(remaining) / static_cast<double>(NumberOfSteps)* static_cast<double>(step))) };
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
        getTimeline().drawLine(dc, region, offset, *mSnap, wxPen{ wxColour{ 164, 164, 164 } });
    }
}

bool Trim::isActive() const
{
    return mActive;
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
        pts minimumsnap = Timeline::SnapDistance + 1; // To ensure that the first snap will update this
        std::vector<pts>::const_iterator itTimeline = mSnapPoints.begin();
        while (itTimeline != mSnapPoints.end())
        {
            pts pts_timeline = *itTimeline;
            pts diff = ptsmouse - pts_timeline;
            if ((abs(diff) <= Timeline::SnapDistance) && (abs(diff) < abs(minimumsnap)))
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
    if (Config::get().read<bool>(Config::sPathTimelineSnapClips))
    {
        std::set<model::IClipPtr> exclude = { originalclip, originalclip->getLink() };
        std::vector<pts> all;
        UtilVector<pts>(all).addElements(getSequence()->getCuts(exclude));
        // Copy everything between [min,max), discard everything else
        mSnapPoints.insert(mSnapPoints.begin(), std::lower_bound(all.begin(), all.end(), min), std::upper_bound(all.begin(), all.end(), max));
    }
    if (Config::get().read<bool>(Config::sPathTimelineSnapCursor))
    {
        mSnapPoints.push_back(getCursor().getLogicalPosition());
    }
    std::sort(mSnapPoints.begin(), mSnapPoints.end());
    std::unique(mSnapPoints.begin(), mSnapPoints.end());
    VAR_DEBUG(originalclip)(min)(max)(mSnapPoints);
}

void Trim::preview()
{
    if (mVideoClip == nullptr)
    {
        return;
    }

    // Always create a new clip. Reusing an existing clip leads to decoding errors sometimes.
    // See [#191]. The decoding errors result in wrong trimming feedback.
    model::VideoClipPtr preview = mVideoClip ? make_cloned<model::VideoClip>(mVideoClip) : nullptr;
    preview->maximize();

    pts diff = mCommand->getDiff();
    ASSERT_LESS_THAN_EQUALS(mStartPositionPreview + diff, preview->getLength());
    pts position(mStartPositionPreview + diff);
    wxSize playerSize = getPlayer()->getVideoSize();
    bool isBeginTrim = mCommand->isBeginTrim();
    bool drawSideBySide = mCommand->isShiftTrim() && mAdjacentBitmap;

    bool completelyTrimmedAway = false;
    if (position == preview->getLength())
    {
        ASSERT(isBeginTrim);
        completelyTrimmedAway = true; // Clip has been trimmed away completely. From the beginning.
    }
    ASSERT_MORE_THAN_EQUALS(position, -1);
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
        mDc.SetBrush(wxBrush{ wxColour{ 0, 0, 0 } });
        mDc.SetPen(wxPen{ wxColour{ 0, 0, 0 } });
        mDc.DrawRectangle(x, 0, w, h);
    }
    else
    {
        preview->moveTo(position);
        model::VideoFramePtr videoFrame = preview->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(w, h)));
        ASSERT(videoFrame); // A frame must be possible, due to the 'completelyTrimmedAway' check above.
        wxBitmapPtr trimmedBmp = videoFrame->getBitmap();
        if (trimmedBmp)
        {
            wxMemoryDC dcBmp(*trimmedBmp);
            mDc.Blit(wxPoint(x, (h - trimmedBmp->GetHeight()) / 2), trimmedBmp->GetSize(), &dcBmp, wxPoint(0, 0));
        }
    }
    mDc.SelectObject(wxNullBitmap);
    getPlayer()->showPreview(bitmap);
}

}} // namespace