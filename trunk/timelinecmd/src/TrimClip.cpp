#include "TrimClip.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "Timeline.h"
#include "Cursor.h"
#include "Track.h"
#include "Sequence.h"
#include "Zoom.h"
#include "EmptyClip.h"
#include "IClip.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimClip::TrimClip(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, MouseOnClipPosition position, bool left)
    :   AClipEdit(sequence)
    ,   mOriginalClip(clip)
    ,   mOriginalLink(mOriginalClip->getLink())
    ,   mClip()
    ,   mLink()
    ,   mTransition(transition)
    ,   mPosition(position)
    ,   mDiff(0)
    ,   mLeft(left)
    ,   mShift(false)
    ,   mMinShiftOtherTrackContent(0)
    ,   mMaxShiftOtherTrackContent(0)
    ,   mSubmitted(false)
{
    VAR_INFO(this)(mClip)(mTransition)(left);
    if (mLeft)
    {
        mCommandName = _("Adjust clip begin point");
    }
    else
    {
        mCommandName = _("Adjust clip end point");
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

void TrimClip::update(bool shift, pts diff)
{
    Revert();

    mShift = shift;
    VAR_INFO(this)(shift)(diff);

    removeTransition();

    determineTrim(diff);

    // This statement is deliberately after removeTransition, since that methods initializes mClip
    // mClip, in turn, is used for preview(). TODO: better solution
    if (mDiff == 0)
    {
        Revert(); // Undo any changes
        return; // Nothing is changed
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

pts TrimClip::getDiff() const
{
    return mDiff;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TrimClip:: determineShiftBoundariesForOtherTracks()
{
    // Do not use mClip and mLink here, since this is called before 'removeTransition' is done.
    mMinShiftOtherTrackContent = (std::numeric_limits<pts>::min)();
    mMaxShiftOtherTrackContent = (std::numeric_limits<pts>::max)();
    pts shiftFrom = mOriginalClip->getLeftPts();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        if (mOriginalClip->getTrack() == track) continue;
        if (mOriginalLink && mOriginalLink->getTrack() == track) continue;
        model::IClipPtr clipAt = track->getClip(shiftFrom);
        // todo handle transitions in other tracks
        // todo this algorithm when trimming under a transition
        // todo this algorithm with more than 1 or two tracks? (or does the max work?)
        mMinShiftOtherTrackContent =
            (clipAt->isA<model::EmptyClip>()) ? std::max<pts>(mMinShiftOtherTrackContent, clipAt->getLeftPts() - shiftFrom) : 0;
        mMaxShiftOtherTrackContent =
            (clipAt->isA<model::EmptyClip>()) ? std::min<pts>(mMaxShiftOtherTrackContent, clipAt->getRightPts() - shiftFrom) : 0;
    }
}

void TrimClip::removeTransition()
{
    mClip = mOriginalClip;
    if (!mShift && mTransition && mDiff != 0)
    {
        model::IClips replacements = unapplyTransition(mTransition);
        ASSERT_MORE_THAN_ZERO(replacements.size());
        if (mLeft)
        {
            mClip = replacements.back();
        }
        else
        {
            mClip = replacements.front();
        }
        ASSERT(!mClip->isA<model::EmptyClip>());
    }
    mLink = mOriginalLink;
    // todo unapply linked transition instead of using originallink
    VAR_INFO(this)(mClip)(mLink);
    ASSERT(mOriginalClip);
    ASSERT(mClip);
    ASSERT(!mOriginalLink || mLink);
}

void TrimClip::reduceSize(model::IClipPtr emptyclip, pts begin, pts end)
{
    ASSERT(emptyclip);
    ASSERT(emptyclip->isA<model::EmptyClip>());
    ASSERT_MORE_THAN_EQUALS_ZERO(begin);
    ASSERT_MORE_THAN_EQUALS_ZERO(end);
    ASSERT_MORE_THAN_EQUALS(emptyclip->getMaxAdjustBegin(),begin);
    ASSERT_LESS_THAN_EQUALS(emptyclip->getMinAdjustEnd(),-end);

    model::IClipPtr clone = make_cloned<model::IClip>(emptyclip);
    if (begin > 0)
    {
        clone->adjustBegin(begin);
    }
    if (end > 0)
    {
        clone->adjustEnd(-end);
    }
    replaceClip(emptyclip, boost::assign::list_of(clone));
}

void TrimClip::determineTrim(pts mousediff)
{
    mDiff = mousediff;

    auto lowerlimit = [this](pts limit) { if (mDiff < limit) { mDiff = limit; } };
    auto upperlimit = [this](pts limit) { if (mDiff > limit) { mDiff = limit; } };

    if (mShift)
    {
        lowerlimit(mMinShiftOtherTrackContent);       // When shift trimming: the contents in other tracks must be able to be shifted accordingly
        upperlimit(mMaxShiftOtherTrackContent);       // When shift trimming: the contents in other tracks must be able to be shifted accordingly
    }

    if (mLeft)
    {
        upperlimit(mClip->getMaxAdjustBegin());    // Clip cannot be trimmed further than the original number of frames
        upperlimit(mLink->getMaxAdjustBegin());    // Clip cannot be trimmed further than the original number of frames in the linked clip

        if (mLink)
        {
            lowerlimit(mClip->getMinAdjustBegin());    // Clip cannot be extended further than the '0'th frame of the underlying video provider.
            lowerlimit(mLink->getMinAdjustBegin());    // Link cannot be extended further than the '0'th frame of the underlying video provider.
        }
    }
    else
    {
        lowerlimit(mClip->getMinAdjustEnd());      // Clip cannot be trimmed further than the original number of frames
        upperlimit(mClip->getMaxAdjustEnd());      // Clip cannot be extended further than the last frame of the underlying video provider.

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
            lowerlimit(mClip->getTrack()->getLeftEmptyArea(mClip));   // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            if (mLink) { lowerlimit(mLink->getTrack()->getLeftEmptyArea(mLink)); }                // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case ClipEnd:
            upperlimit(mClip->getTrack()->getRightEmptyArea(mClip));  // When not shift trimming: extended clip must fit into the available empty area in front of the clip
            if (mLink) { upperlimit(mLink->getTrack()->getRightEmptyArea(mLink)); }                          // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionRightClipBegin:
            lowerlimit(mClip->getTrack()->getLeftEmptyArea(mClip->getPrev()));   // When not shift trimming: extended clip must fit into the available empty area in front of the clip. For in-out transitions this will always be 0...
            if (mLink) { lowerlimit(mLink->getTrack()->getLeftEmptyArea(mLink)); }                // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionLeftClipEnd:
            upperlimit(mClip->getTrack()->getRightEmptyArea(mClip->getNext())); // When not shift trimming: extended clip must fit into the available empty area in front of the transition. For in-out transitions this will always be 0...
            if (mLink) { upperlimit(mLink->getTrack()->getRightEmptyArea(mLink)); }                          // When not shift trimming: extended link must fit into the available empty area in front of the link
            break;
        case TransitionBegin:
        case TransitionEnd:
            // TODO
        case ClipInterior:
        case TransitionLeftClipInterior:
        case TransitionInterior:
        case TransitionRightClipInterior:
        default:
            FATAL("Illegal clip position");
        }
    }

    VAR_INFO(this)(mDiff);
}

void TrimClip::applyTrim()
{
    model::IClipPtr newclip;
    model::IClipPtr newlink;

    newclip = make_cloned<model::IClip>(mClip);
    if (mLeft)
    {
        newclip->adjustBegin(mDiff);
    }
    else
    {
        newclip->adjustEnd(mDiff);
    }

    if (mLink)
    {
        // TODO NIY: What if the link is 'shifted' wrt original clip?
        //ASSERT_EQUALS(mClip->getLeftPts(),linked->getLeftPts());
        //ASSERT_EQUALS(mClip->getRightPts(),linked->getRightPts());
        // This is already seen when trimming with transitions

        newlink = make_cloned<model::IClip>(mLink);
        if (mLeft)
        {
            newlink->adjustBegin(mDiff);
        }
        else
        {
            newlink->adjustEnd(mDiff);
        }
    }

    auto makelist = [](model::IClipPtr clip) -> model::IClips
    {
        if (clip->getLength() == 0)
        {
            // If the clip or its link is resized to 0 frames, then replace with "nothing"
            return model::IClips();
        }
        return boost::assign::list_of(clip);
    };

    model::IClips replaceclip = makelist(newclip);
    model::IClips replacelink = makelist(newlink);

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

        // Note that the state class is responsible for checking that there is enough space
        // in case of moving clips to the left. todo why this statement??

        // \todo what if the linked clip is more to the left. Then that position should
        // be used for shifting other tracks?
        //ASSERT_EQUALS(mClip->getLeftPts(),linked->getLeftPts());
        //ASSERT_EQUALS(mClip->getRightPts(),linked->getRightPts());

        shiftAllTracks(mClip->getLeftPts(), -mDiff,  exclude);
    }
    else
    {
        if (mLeft)
        {
            if (mDiff > 0) // Reduce: Move clip begin point to the right
            {
                // Add empty clip in front of new clip: new clip is shorter than original clip and the frames should maintain their position.
                replaceclip.push_front(boost::make_shared<model::EmptyClip>(mDiff));
                replacelink.push_front(boost::make_shared<model::EmptyClip>(mDiff));
            }
            else // (mDiff < 0) // Enlarge: Move clip begin point to the left
            {
                ASSERT(mClip->getPrev()->isA<model::EmptyClip>());
                reduceSize(mClip->getPrev(), 0, -mDiff);
                if (mLink)
                {
                    ASSERT(mLink->getPrev()->isA<model::EmptyClip>());
                    reduceSize(mLink->getPrev(), 0, -mDiff);
                }
            }
        }
        else // !mLeft
        {
            if (mDiff < 0) // Reduce: Move clip end point to the left
            {
                // Add empty clip after new clip: new clip is shorter than original clip and the frames should maintain their position.
                replaceclip.push_back(boost::make_shared<model::EmptyClip>(-mDiff));
                replacelink.push_back(boost::make_shared<model::EmptyClip>(-mDiff));
            }
            else // (mDiff > 0) // Enlarge: Move clip end point to the right
            {
                ASSERT(mClip->getNext()->isA<model::EmptyClip>());
                reduceSize(mClip->getNext(), mDiff, 0);
                if (mLink)
                {
                    ASSERT(mLink->getNext()->isA<model::EmptyClip>());
                    reduceSize(mLink->getNext(), mDiff, 0);
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
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mOriginalClip << '|' << obj.mClip << '|' << obj.mDiff << '|' << obj.mLeft << '|' << obj.mShift;
    return os;
}

}}} // namespace