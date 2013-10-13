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

#include "TrimClip.h"

#include "Cursor.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "Keyboard.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimClip::TrimClip(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, MouseOnClipPosition position)
    :   AClipEdit(sequence)
    ,   mOriginalClip(clip)
    ,   mOriginalLink(mOriginalClip->getLink())
    ,   mClip()
    ,   mLink()
    ,   mNewClip(mOriginalClip)
    ,   mNewLink(mOriginalLink)
    ,   mTransition(transition)
    ,   mLinkTransition()
    ,   mLinkIsPartOfTransition(false)
    ,   mClipIsPartOfTransition(false)
    ,   mTrim(0)
    ,   mShift(false)
    ,   mPosition(position)
    ,   mShiftStart(0)
{
    VAR_INFO(this)(mClip)(mTransition);
    mCommandName = _("Adjust length");
    ASSERT(!mOriginalClip->isA<model::EmptyClip>());
}

TrimClip::~TrimClip()
{
    if (!isInitialized())
    {
        Revert();
        getTimeline().getSelection().change(boost::assign::list_of(mOriginalClip)(mOriginalLink));
    }
}

void TrimClip::update(pts diff, bool shift)
{
    VAR_DEBUG(diff)(shift);
    mNewClip.reset();
    mNewLink.reset();
    Revert();

    mCommandName =
        _("Adjust ") +
        (mOriginalClip->isA<model::Transition>() ? _("transition ") : _("clip ")) +
        (isBeginTrim() ? _("begin") : _("end")) +
        _(" point");

    mLinkTransition.reset();
    if (mOriginalLink)
    {
        mLinkTransition = isBeginTrim() ? mOriginalLink->getInTransition() : mOriginalLink->getOutTransition();
    }

    mShift =
        shift ? true :                                       // Some trim operations are not directly user triggered but a result of - for instance - making room for a transition.
        mOriginalClip->isA<model::Transition>() ? false :    // When trimming a transition, shift does nothing. Reset here to avoid having to deal with that (if-then-else) later on.
        mShift = getTimeline().getKeyboard().getShiftDown(); // Default: shift trim when shift key is down.
    VAR_INFO(this)(mShift)(diff);

    removeTransition();

    TrimLimit limits = determineBoundaries(getSequence(), mClip, mLink, mPosition, mShift);

    mTrim = diff;
    if (mTrim < limits.Min) { mTrim = limits.Min; }
    if (mTrim > limits.Max) { mTrim = limits.Max; }

    // This statement is deliberately after removeTransition, since that method initializes mClip.
    // mClip, in turn, is used for preview().
    if (mTrim == 0)
    {
        mNewClip = mOriginalClip;
        mNewLink = mOriginalLink;
        Revert(); // Undo any changes
    }
    else
    {
        applyTrim();
    }

    getTimeline().getSelection().change(boost::assign::list_of(mNewClip)(mNewLink));
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimClip::initialize()
{
    // Already done in 'update'. This is called afterwards when the command is submitted.
}

void TrimClip::doExtraAfter()
{
    getTimeline().getSelection().change(boost::assign::list_of(mNewClip)(mNewLink));
}

void TrimClip::undoExtraAfter()
{
    getTimeline().getSelection().change(boost::assign::list_of(mOriginalClip)(mOriginalLink));
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr TrimClip::getOriginalClip() const
{
    return mOriginalClip;
}

model::IClipPtr TrimClip::getOriginalLink() const
{
    return mOriginalLink;
}

model::IClipPtr TrimClip::getNewClip() const
{
    return mNewClip;
}

model::IClipPtr TrimClip::getNewLink() const
{
    return mNewLink;
}

// static
bool TrimClip::isBeginTrim(MouseOnClipPosition position)
{
    switch (position)
    {
    case ClipBegin:
    case TransitionRightClipBegin:
    case TransitionBegin:
        return true;
    case ClipEnd:
    case TransitionLeftClipEnd:
    case TransitionEnd:
        return false;
    case ClipInterior:
    case TransitionLeftClipInterior:
    case TransitionInterior:
    case TransitionRightClipInterior:
    default:
        FATAL("Illegal clip position");
    }
    return false;
}

bool TrimClip::isBeginTrim() const
{
    return isBeginTrim(mPosition);
}

pts TrimClip::getDiff() const
{
    return mTrim;
}

pts TrimClip::getShiftStart() const
{
    return mShiftStart;
}

// static
TrimClip::TrimLimit TrimClip::determineBoundaries(model::SequencePtr sequence, model::IClipPtr clip, model::IClipPtr link, MouseOnClipPosition position, bool shift)
{
    ASSERT(clip && clip->getTrack());
    ASSERT_IMPLIES(link,link->getTrack());

    TrimLimit result;

    auto lowerlimit = [](TrimLimit& scenario, pts limit) { if (scenario.Min < limit) { scenario.Min = limit; } };
    auto upperlimit = [](TrimLimit& scenario, pts limit) { if (scenario.Max > limit) { scenario.Max = limit; } };

    // Boundaries imposed by the lengths/offset of the clip (and it's link):
    if (isBeginTrim(position))
    {
        // Boundaries at begin of clip:
        lowerlimit(result, clip->getMinAdjustBegin());        // Clip cannot be extended further than the '0'th frame of the underlying video provider.
        upperlimit(result, clip->getMaxAdjustBegin());        // Clip cannot be trimmed further than the original number of frames
        if (link)
        {
            lowerlimit(result, link->getMinAdjustBegin());    // Link cannot be extended further than the '0'th frame of the underlying video provider.
            upperlimit(result, link->getMaxAdjustBegin());    // Clip cannot be trimmed further than the original number of frames in the linked clip
        }
    }
    else
    {
        // Boundaries at end of clip:
        lowerlimit(result, clip->getMinAdjustEnd());          // Clip cannot be trimmed further than the original number of frames
        upperlimit(result, clip->getMaxAdjustEnd());          // Clip cannot be extended further than the last frame of the underlying video provider.
        if (link)
        {
            lowerlimit(result, link->getMinAdjustEnd());      // Clip cannot be trimmed further than the original number of frames in the linked clip
            upperlimit(result, link->getMaxAdjustEnd());      // Link cannot be extended further than the '0'th frame of the underlying video provider.
        }
    }

    if (!shift)
    {
        // When not shift trimming the extended clip must be able to fit within the empty area before/after the clip:
        switch (position)
        {
        case ClipBegin:
            lowerlimit(result, clip->getTrack()->getLeftEmptyArea(clip));                 // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            if (link) { lowerlimit(result, link->getTrack()->getLeftEmptyArea(link)); }   // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case ClipEnd:
            upperlimit(result, clip->getTrack()->getRightEmptyArea(clip));                // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            if (link) { upperlimit(result, link->getTrack()->getRightEmptyArea(link)); }  // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionRightClipBegin:
            lowerlimit(result, clip->getTrack()->getLeftEmptyArea(clip->getPrev()));      // When not shift trimming: extended clip must fit into the available empty area in front of the clip. For in-out transitions this will always be 0...
            if (link) { lowerlimit(result, link->getTrack()->getLeftEmptyArea(link)); }   // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionLeftClipEnd:
            upperlimit(result, clip->getTrack()->getRightEmptyArea(clip->getNext()));     // When not shift trimming: extended clip must fit into the available empty area in front of the transition. For in-out transitions this will always be 0...
            if (link) { upperlimit(result, link->getTrack()->getRightEmptyArea(link)); }  // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionBegin:
        case TransitionEnd:
            // There is no empty area involved. There is either a clip left and/or right of the transition. If one of these is not present (in-only, or out-only transition) then the clip
            // itselves cannot be enlarged on 'the other side'.
            break;
        case ClipInterior:
        case TransitionLeftClipInterior:
        case TransitionInterior:
        case TransitionRightClipInterior:
        default:
            FATAL("Illegal clip position");
        }
    }
    else // (shift)
    {
        // When shift trimming the contents in other tracks must be able to be shifted accordingly
        pts minShiftOtherTrackContent = (std::numeric_limits<pts>::min)();
        pts maxShiftOtherTrackContent = (std::numeric_limits<pts>::max)();
        pts shiftFrom = (isBeginTrim(position)) ? clip->getLeftPts() : clip->getRightPts();
        for ( model::TrackPtr track : sequence->getTracks() )
        {
            if (clip->getTrack() == track) continue;
            if (link && link->getTrack() == track) continue;
            model::IClipPtr clipInOtherTrack = track->getClip(shiftFrom);

            if (!clipInOtherTrack)
            {
                // There is no clip at the given position (beyond track length) then the only restriction is the length of the track.
                if (isBeginTrim(position))
                {
                    // No change to mMaxShiftOtherTrackContent
                }
                else
                {
                    minShiftOtherTrackContent = std::max<pts>(minShiftOtherTrackContent, track->getLength() - shiftFrom);
                }
            }
            else
            {
                if (!clipInOtherTrack->isA<model::EmptyClip>())
                {
                    // No shift allowed if there's a track that has 'filled space' at the shift position
                    minShiftOtherTrackContent = 0;
                    maxShiftOtherTrackContent = 0;
                }
                else
                {
                    if (isBeginTrim(position))
                    {
                        maxShiftOtherTrackContent = std::min<pts>(maxShiftOtherTrackContent, clipInOtherTrack->getRightPts() - shiftFrom);
                    }
                    else
                    {
                        minShiftOtherTrackContent = std::max<pts>(minShiftOtherTrackContent, clipInOtherTrack->getLeftPts() - shiftFrom);
                    }
                }
            }
        }

        lowerlimit(result, minShiftOtherTrackContent);        // When shift trimming: the contents in other tracks must be able to be shifted accordingly
        upperlimit(result, maxShiftOtherTrackContent);        // When shift trimming: the contents in other tracks must be able to be shifted accordingly
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TrimClip::removeTransition()
{
    auto unapplyIfNeeded = [this](model::IClipPtr clip, model::TransitionPtr transition) -> model::IClipPtr
    {
        model::IClipPtr result = clip;
        // Do not use mTrim here. That is initialized AFTER this method!
        if (!mShift && transition && transition->getLeft() > 0 && transition->getRight() > 0)
        {
            // Only in case
            // - No shift trim is applied
            // - the clip is part of a transition (and that side of the transition is trimmed)
            // - the transition is an InOut transition
            // it must be unapplied.
            model::IClips replacements = unapplyTransition(transition);
            ASSERT_MORE_THAN_ZERO(replacements.size());
            if (isBeginTrim())
            {
                result = replacements.back();
            }
            else
            {
                result = replacements.front();
            }
        }
        ASSERT(!result->isA<model::EmptyClip>());
        return result;
    };

    mClip = unapplyIfNeeded(mOriginalClip,mTransition);
    mClipIsPartOfTransition = mClip->getInTransition() || mClip->getOutTransition();
    if (mOriginalLink)
    {
        mLink = unapplyIfNeeded(mOriginalLink,mLinkTransition);
        mLinkIsPartOfTransition = mLink->getInTransition() || mLink->getOutTransition();
    }

    VAR_INFO(this)(mClip)(mLink)(mClipIsPartOfTransition)(mLinkIsPartOfTransition);
    ASSERT(mOriginalClip);
    ASSERT(mClip);
    ASSERT(!mOriginalLink || mLink);
}

void TrimClip::adjust(model::IClipPtr clip, pts begin, pts end)
{
    ASSERT(clip);
    ASSERT(!clip->isA<model::Transition>());
    ASSERT((begin != 0) || (end != 0));

    model::IClipPtr clone = make_cloned<model::IClip>(clip);
    if (begin != 0)
    {
        clone->adjustBegin(begin);
    }
    if (end != 0)
    {
        clone->adjustEnd(end);
    }
    replaceClip(clip, boost::assign::list_of(clone));
}

void TrimClip::applyTrim()
{
    ASSERT_NONZERO(mTrim);
    auto makeTrimmedClone = [this](model::IClipPtr clip, bool clipIsPartOfTransition) -> model::IClips
    {
        // Make a clone and adjust that
        model::IClipPtr clone = make_cloned<model::IClip>(clip);
        if (isBeginTrim())
        {
            clone->adjustBegin(mTrim);
        }
        else
        {
            clone->adjustEnd(mTrim);
        }
        if ((clone->getLength() == 0) && !clipIsPartOfTransition)
        {
            // If the clip or its link is resized to 0 frames, then replace with "nothing"
            //
            // An exception is a clip that is part of a transition:
            // The clip may be reduced to 'zero' frames. However, the clip must still be
            // a part of the timeline (with length 0), since it is used by the transition
            // that comes directly after it.
            //
            // Note that transitions themselves ARE replaced with "nothing" if their length
            // becomes 0 after trimming.
            return model::IClips();
        }
        return boost::assign::list_of(clone);
    };

    model::IClips replaceclip = makeTrimmedClone(mClip,mClipIsPartOfTransition);
    model::IClips replacelink = mLink ? makeTrimmedClone(mLink,mLinkIsPartOfTransition) : model::IClips();
    mNewClip = (replaceclip.size() > 0) ? replaceclip.front() : model::IClipPtr();
    mNewLink = (replacelink.size() > 0) ? replacelink.front() : model::IClipPtr();

    // Now adjust other clips to ensure that the rest of the track(s) are positioned correctly.
    // That means enlarging/reducing empty space in front of/after the clip(s) being changed
    // as well as enlarging/reducing clips as a result of changing a transition's size.
    if (mOriginalClip->isA<model::Transition>())
    {
        if (isBeginTrim())
        {
            ASSERT(!mClip->getPrev()->isA<model::EmptyClip>());
            adjust(mClip->getPrev(), 0, mTrim); // Adjust the end point of the previous clip in line with the transition size change
        }
        else // !isBeginTrim()
        {
            ASSERT(!mClip->getNext()->isA<model::EmptyClip>());
            adjust(mClip->getNext(), mTrim, 0); // Adjust the begin point of the next clip in line with the transition size change
        }
    }
    else
    {
        if (mShift)
        {
            if (isBeginTrim())
            {
                mShiftStart = mClip->getLeftPts();
            }
            else
            {
                mShiftStart = mClip->getRightPts();
            }
        }
        if (mShift)
        {
            // Move clips in other tracks (that's the 'shift') - and only in other tracks
            // The clips in the same track as mClip and linked are shifted automatically
            // because of the enlargement/reduction of these two clips.
            model::Tracks exclude = boost::assign::list_of(mClip->getTrack());
            if (mLink) { exclude.push_back(mLink->getTrack()); }

            if (isBeginTrim())
            {
                shiftAllTracks(mClip->getLeftPts(), -mTrim,  exclude);
            }
            else
            {
                shiftAllTracks(mClip->getRightPts(), mTrim,  exclude);
            }
        }
        else // ! mShift
        {
            // Adjust empty space before/after the clips being changed. This is required to keep all
            // cuts in the track at the exact same position (with the exception of the cut being moved).
            model::TransitionPtr intransition = mClip->getInTransition();
            model::TransitionPtr outtransition = mClip->getOutTransition();
            if (isBeginTrim())
            {
                if (mTrim > 0) // Reduce: Move clip begin point to the right
                {
                    auto adjustEmptySpace = [this](model::IClipPtr clip, model::IClips& replacement)
                    {
                        model::TransitionPtr intransition = clip->getInTransition(); // An in-only-transition in front of the clip must be 'moved along'
                        if (intransition)
                        {
                            ASSERT_ZERO(intransition->getLeft())(intransition); // If the transition was in-out then it should have been removed at the beginning of the trim operation
                            removeClip(intransition);                           // Remove from the timeline...
                            replacement.push_front(make_cloned(intransition));  // ...and add again in the correct position (clone is added to avoid issues when expanding the link replacements)
                        }
                        replacement.push_front(boost::make_shared<model::EmptyClip>(mTrim)); // Add empty space to keep all clips after the trim in exact the same position
                    };
                    adjustEmptySpace(mClip, replaceclip);
                    if (mLink) { adjustEmptySpace(mLink, replacelink); }
                }
                else // (mTrim < 0) // Enlarge: Move clip begin point to the left
                {
                    auto adjustEmptySpace = [this](model::IClipPtr clip, model::IClips& replacement)
                    {
                        model::TransitionPtr intransition = clip->getInTransition(); // An in-only-transition in front of the clip must be 'moved along'
                        model::IClipPtr emptyspace = clip->getPrev();
                        if (intransition)
                        {
                            ASSERT_ZERO(intransition->getLeft())(intransition); // If the transition was in-out then it should have been removed at the beginning of the trim operation
                            emptyspace = intransition->getPrev();
                        }
                        ASSERT(emptyspace); // There had to be room for enlarging
                        ASSERT(emptyspace->isA<model::EmptyClip>());
                        adjust(emptyspace, 0, mTrim);
                    };
                    adjustEmptySpace(mClip, replaceclip);
                    if (mLink) { adjustEmptySpace(mLink, replacelink); }
                }
            }
            else // !mBeginTrim
            {
                if (mTrim < 0) // Reduce: Move clip end point to the left
                {
                    auto adjustEmptySpace = [this](model::IClipPtr clip, model::IClips& replacement)
                    {
                        model::TransitionPtr outtransition = clip->getOutTransition(); // An out-only-transition after the clip must be 'moved along'
                        if (outtransition)
                        {
                            ASSERT_ZERO(outtransition->getRight())(outtransition); // If the transition was in-out then it should have been removed at the beginning of the trim operation
                            removeClip(outtransition);                             // Remove from the timeline...
                            replacement.push_back(make_cloned(outtransition));     // ...and add again in the correct position (clone is added to avoid issues when expanding the link replacements)
                        }
                        replacement.push_back(boost::make_shared<model::EmptyClip>(-mTrim)); // Add empty space to keep all clips after the trim in exact the same position
                    };
                    adjustEmptySpace(mClip, replaceclip);
                    if (mLink) { adjustEmptySpace(mLink, replacelink); }
                }
                else // (mTrim > 0) // Enlarge: Move clip end point to the right
                {
                    auto adjustEmptySpace = [this](model::IClipPtr clip, model::IClips& replacement)
                    {
                        model::TransitionPtr outtransition = clip->getOutTransition(); // An out-only-transition after the clip must be 'moved along'
                        model::IClipPtr emptyspace = clip->getNext();
                        if (outtransition)
                        {
                            ASSERT_ZERO(outtransition->getRight())(outtransition); // If the transition was in-out then it should have been removed at the beginning of the trim operation
                            emptyspace = outtransition->getNext();
                        }
                        if (emptyspace)
                        {
                            ASSERT(emptyspace->isA<model::EmptyClip>());
                            adjust(emptyspace, mTrim, 0);
                        }
                        // else: Trimming the last clip in the track (There is not neccessarily an empty clip after it)
                    };
                    adjustEmptySpace(mClip, replaceclip);
                    if (mLink) { adjustEmptySpace(mLink, replacelink); }
                }
            }
        }
    }

    replaceClip(mClip, replaceclip);
    if (mLink)
    {
        replaceClip(mLink, replacelink);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TrimClip& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mOriginalClip << '|' << obj.mClip << '|' << obj.mTrim << '|' << '|' << obj.mShift << '|' << obj.mPosition;
    return os;
}
}}} // namespace