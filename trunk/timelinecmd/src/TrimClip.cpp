#include "TrimClip.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "Cursor.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
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
    ,   mTransition(transition)
    ,   mLinkTransition()
    ,   mLinkIsPartOfTransition(false)
    ,   mClipIsPartOfTransition(false)
    ,   mPosition(position)
    ,   mTrim(0)
    ,   mShift(false)
    ,   mMinShiftOtherTrackContent(0)
    ,   mMaxShiftOtherTrackContent(0)
    ,   mSubmitted(false)
{
    VAR_INFO(this)(mClip)(mTransition)(mPosition);
    wxString cliptype = mOriginalClip->isA<model::Transition>() ? _("transition") : _("clip");
    wxString trimpos = isBeginTrim() ? _("begin") : _("end");
    mCommandName = _("Adjust ") + cliptype + " " + trimpos + _(" point");
    if (isBeginTrim())
    {
        mLinkTransition = mOriginalLink ? mOriginalLink->getInTransition() : model::TransitionPtr();
    }
    else
    {
        mLinkTransition = mOriginalLink ? mOriginalLink->getOutTransition() : model::TransitionPtr();
    }

    determineShiftBoundariesForOtherTracks();
}

TrimClip::~TrimClip()
{
    if (!mSubmitted)
    {
        Revert();
    }
}

void TrimClip::update(pts diff)
{
    Revert();

    mShift = wxGetMouseState().ShiftDown();
    if (mOriginalClip->isA<model::Transition>())
    {
        // When trimming a transition, shift does nothing.
        // Reset here to avoid having to deal with that (if-then-else)
        // later on.
        mShift = false;
    }
    VAR_INFO(this)(mShift)(diff);

    removeTransition();

    determineTrim(diff);

    // This statement is deliberately after removeTransition, since that methods initializes mClip
    // mClip, in turn, is used for preview().
    if (mTrim == 0)
    {
        Revert(); // Undo any changes
        return; // Nothing is changed (this avoids having to check 'if (trim == 0)' throughout applyTrim().
    }

    applyTrim();
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimClip::initialize()
{
    // Does nothing, since that's already done in prepare.
    mSubmitted = true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr TrimClip::getClip() const
{
    return mClip;
}

bool TrimClip::isBeginTrim() const
{
    bool result = false;

    switch (mPosition)
    {
    case ClipBegin:
    case TransitionRightClipBegin:
    case TransitionBegin:
        result = true;
        break;
    case ClipEnd:
    case TransitionLeftClipEnd:
    case TransitionEnd:
        break;
    case ClipInterior:
    case TransitionLeftClipInterior:
    case TransitionInterior:
    case TransitionRightClipInterior:
    default:
        FATAL("Illegal clip position");
    }
    return result;
}

pts TrimClip::getDiff() const
{
    return mTrim;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TrimClip:: determineShiftBoundariesForOtherTracks()
{
    // Do not use mClip and mLink here, since this is called before 'removeTransition' is done.
    mMinShiftOtherTrackContent = (std::numeric_limits<pts>::min)();
    mMaxShiftOtherTrackContent = (std::numeric_limits<pts>::max)();
    pts shiftFrom = (isBeginTrim()) ? mOriginalClip->getLeftPts() : mOriginalClip->getRightPts(); // todo test begin + end trim (rightpts)
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        if (mOriginalClip->getTrack() == track) continue;
        if (mOriginalLink && mOriginalLink->getTrack() == track) continue;
        model::IClipPtr clipInOtherTrack = track->getClip(shiftFrom);

        if (!clipInOtherTrack)
        {
            // There is no clip at the given position (beyond track length) then the only restriction
            // is the length of the track.
            pts tracklength = track->getLength();
            if (isBeginTrim())
            {
                    // todo test
                mMaxShiftOtherTrackContent = std::min<pts>(mMaxShiftOtherTrackContent, tracklength - shiftFrom);
            }
            else
            {
                    // todo test
                mMinShiftOtherTrackContent = std::max<pts>(mMinShiftOtherTrackContent, tracklength - shiftFrom);
            }
        }
        else
        {
            if (!clipInOtherTrack->isA<model::EmptyClip>())
            {
                    // todo test
                // No shift allowed if there's a track that has 'filled space' at the shift position
                mMinShiftOtherTrackContent = 0;
                mMaxShiftOtherTrackContent = 0;
            }
            else
            {
                if (isBeginTrim())
                {
                    // todo test
                    mMaxShiftOtherTrackContent = std::min<pts>(mMaxShiftOtherTrackContent, clipInOtherTrack->getRightPts() - shiftFrom);
                }
                else
                {
                    // todo test
                    mMinShiftOtherTrackContent = std::max<pts>(mMinShiftOtherTrackContent, clipInOtherTrack->getLeftPts() - shiftFrom);
                }
            }
        }

        // todo handle transitions in other tracks
        // todo this algorithm with more than 1 or two tracks? (or does the max work?)
        // todo this fails if one of the other tracks has no clips yet/...
    }
}

void TrimClip::removeTransition()
{
    auto unapplyIfNeeded = [this](model::IClipPtr clip, model::TransitionPtr transition) -> model::IClipPtr
    {
        model::IClipPtr result = clip;
        if (!mShift && transition && mTrim != 0)
        {
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

void TrimClip::determineTrim(pts mousediff)
{
    mTrim = mousediff;

    auto lowerlimit = [this](pts limit) { if (mTrim < limit) { mTrim = limit; } };
    auto upperlimit = [this](pts limit) { if (mTrim > limit) { mTrim = limit; } };

    if (mShift)
    {
        lowerlimit(mMinShiftOtherTrackContent);        // When shift trimming: the contents in other tracks must be able to be shifted accordingly
        upperlimit(mMaxShiftOtherTrackContent);        // When shift trimming: the contents in other tracks must be able to be shifted accordingly
    }

    if (isBeginTrim())
    {
        lowerlimit(mClip->getMinAdjustBegin());        // Clip cannot be extended further than the '0'th frame of the underlying video provider.
        upperlimit(mClip->getMaxAdjustBegin());        // Clip cannot be trimmed further than the original number of frames

        if (mLink)
        {
            lowerlimit(mLink->getMinAdjustBegin());    // Link cannot be extended further than the '0'th frame of the underlying video provider.
            upperlimit(mLink->getMaxAdjustBegin());    // Clip cannot be trimmed further than the original number of frames in the linked clip
        }
    }
    else
    {
        lowerlimit(mClip->getMinAdjustEnd());          // Clip cannot be trimmed further than the original number of frames
        upperlimit(mClip->getMaxAdjustEnd());          // Clip cannot be extended further than the last frame of the underlying video provider.

        if (mLink)
        {
            lowerlimit(mLink->getMinAdjustEnd());      // Clip cannot be trimmed further than the original number of frames in the linked clip
            upperlimit(mLink->getMaxAdjustEnd());      // Link cannot be extended further than the '0'th frame of the underlying video provider.
        }
    }

    if (!mShift)
    {
        switch (mPosition)
        {
        case ClipBegin:
            lowerlimit(mClip->getTrack()->getLeftEmptyArea(mClip));                 // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            if (mLink) { lowerlimit(mLink->getTrack()->getLeftEmptyArea(mLink)); }  // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case ClipEnd:
            upperlimit(mClip->getTrack()->getRightEmptyArea(mClip));                // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            if (mLink) { upperlimit(mLink->getTrack()->getRightEmptyArea(mLink)); } // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionRightClipBegin:
            lowerlimit(mClip->getTrack()->getLeftEmptyArea(mClip->getPrev()));      // When not shift trimming: extended clip must fit into the available empty area in front of the clip. For in-out transitions this will always be 0...
            if (mLink) { lowerlimit(mLink->getTrack()->getLeftEmptyArea(mLink)); }  // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionLeftClipEnd:
            upperlimit(mClip->getTrack()->getRightEmptyArea(mClip->getNext()));     // When not shift trimming: extended clip must fit into the available empty area in front of the transition. For in-out transitions this will always be 0...
            if (mLink) { upperlimit(mLink->getTrack()->getRightEmptyArea(mLink)); } // When not shift trimming: extended link must fit into the available empty area in front of the link
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

    VAR_INFO(this)(mTrim);
}

void TrimClip::applyTrim()
{
    ASSERT_NONZERO(mTrim);
    auto trim = [this](model::IClipPtr clip, bool clipIsPartOfTransition) -> model::IClips
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

    model::IClips replaceclip = trim(mClip,mClipIsPartOfTransition);
    model::IClips replacelink = mLink ? trim(mLink,mLinkIsPartOfTransition) : model::IClips();

    if (mOriginalClip->isA<model::Transition>())
    {
        if (isBeginTrim())
        {
            // Adjust the end point of the previous clip in line with the transition size change
            ASSERT(!mClip->getPrev()->isA<model::EmptyClip>());
            adjust(mClip->getPrev(), 0, mTrim);
        }
        else // !isBeginTrim()
        {
            // Adjust the begin point of the next clip in line with the transition size change
            ASSERT(!mClip->getNext()->isA<model::EmptyClip>());
            adjust(mClip->getNext(), mTrim, 0);
        }
    }
    else
    {
        if (mShift)
        {
            // Move clips in other tracks (that's the 'shift') - and only in other tracks
            // The clips in the same track as mClip and linked are shifted automatically
            // because of the enlargement/reduction of these two clips.
            model::Tracks exclude = boost::assign::list_of(mClip->getTrack());
            if (mLink)
            {
                exclude.push_back(mLink->getTrack());
            }

            shiftAllTracks(mClip->getLeftPts(), -mTrim,  exclude);
        }
        else
        {
            if (isBeginTrim())
            {
                if (mTrim > 0) // Reduce: Move clip begin point to the right
                {
                    // Add empty clip in front of new clip: new clip is shorter than original clip and the frames should maintain their position.
                    replaceclip.push_front(boost::make_shared<model::EmptyClip>(mTrim));
                    replacelink.push_front(boost::make_shared<model::EmptyClip>(mTrim));
                }
                else // (mTrim < 0) // Enlarge: Move clip begin point to the left
                {
                    ASSERT(mClip->getPrev()->isA<model::EmptyClip>());
                    adjust(mClip->getPrev(), 0, mTrim);
                    if (mLink)
                    {
                        ASSERT(mLink->getPrev()->isA<model::EmptyClip>());
                        adjust(mLink->getPrev(), 0, mTrim);
                    }
                }
            }
            else // !isBeginTrim()
            {
                if (mTrim < 0) // Reduce: Move clip end point to the left
                {
                    // Add empty clip after new clip: new clip is shorter than original clip and the frames should maintain their position.
                    replaceclip.push_back(boost::make_shared<model::EmptyClip>(-mTrim));
                    replacelink.push_back(boost::make_shared<model::EmptyClip>(-mTrim));
                }
                else // (mTrim > 0) // Enlarge: Move clip end point to the right
                {
                    ASSERT(mClip->getNext()->isA<model::EmptyClip>());
                    adjust(mClip->getNext(), mTrim, 0);
                    if (mLink)
                    {
                        ASSERT(mLink->getNext()->isA<model::EmptyClip>());
                        adjust(mLink->getNext(), mTrim, 0);
                    }
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
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mOriginalClip << '|' << obj.mClip << '|' << obj.mTrim << '|' << obj.mPosition << '|' << obj.mShift;
    return os;
}
}}} // namespace