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

TrimClip::TrimClip(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, pts diff, bool left, bool shift)
    :   AClipEdit(sequence)
    ,   mClip(clip)
    ,   mTransition(transition)
    ,   mDiff(diff)
    ,   mLeft(left)
    ,   mShift(shift)
{
    VAR_INFO(this)(mClip)(mTransition)(mDiff)(mLeft)(mShift);
    ASSERT_NONZERO(mDiff); // Useless to add an action to the undo list, when there is no change. Furthermore, this ensures that not a transition is unapplied although no actual trim was done.
    if (mLeft)
    {
        mCommandName = _("Adjust clip begin point");
    }
    else
    {
        mCommandName = _("Adjust clip end point");
    }
}

TrimClip::~TrimClip()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimClip::initialize()
{
    VAR_INFO(this);
    model::IClipPtr newclip;
    model::IClipPtr newlink;

    // todo bug: start application, make sequence, shift click clip four. all first four clips selected!

    model::IClipPtr clip = mClip;
    model::IClipPtr linked = mClip->getLink();

    if (!mShift && mTransition)
    {
        model::IClips replacements = unapplyTransition(mTransition);
        ASSERT_MORE_THAN_ZERO(replacements.size());
        if (mLeft)
        {
            clip = replacements.back();
        }
        else
        {
            clip = replacements.front();
        }
        ASSERT(!clip->isA<model::EmptyClip>());
    }

    newclip = make_cloned<model::IClip>(clip);
    if (mLeft)
    {
        newclip->adjustBegin(mDiff);
    }
    else
    {
        newclip->adjustEnd(mDiff);
    }

    if (linked)
    {
        // TODO NIY: What if the link is 'shifted' wrt original clip?
        //ASSERT_EQUALS(mClip->getLeftPts(),linked->getLeftPts());
        //ASSERT_EQUALS(mClip->getRightPts(),linked->getRightPts());
        // This is already seen when trimming with transitions

        newlink = make_cloned<model::IClip>(linked);
        if (mLeft)
        {
            newlink->adjustBegin(mDiff);
        }
        else
        {
            newlink->adjustEnd(mDiff);
        }
    }

    model::IClips replace = boost::assign::list_of(newclip);
    model::IClips replacelink = boost::assign::list_of(newlink);

    // If the clip or its link is resized to 0 frames, then replace with "nothing"
    ASSERT_MORE_THAN_EQUALS_ZERO(newclip->getLength());
    ASSERT_MORE_THAN_EQUALS_ZERO(newlink->getLength());
    if (newclip->getLength() == 0)
    {
        replace.clear();
    }
    if (newlink->getLength() == 0)
    {
        replace.clear();
    }

    if (mShift)
    {
        // Move clips in other tracks (that's the 'shift') - and only in other tracks
        // The clips in the same track as mClip and linked are shifted automatically
        // because of the enlargement/reduction of these two clips.
        model::Tracks exclude = boost::assign::list_of(clip->getTrack());
        if (linked)
        {
            exclude.push_back(linked->getTrack());
        }

        // Note that the state class is responsible for checking that there is enough space
        // in case of moving clips to the left.

        // \todo what if the linked clip is more to the left. Then that position should
        // be used for shifting other tracks?
        //ASSERT_EQUALS(mClip->getLeftPts(),linked->getLeftPts());
        //ASSERT_EQUALS(mClip->getRightPts(),linked->getRightPts());

        shiftAllTracks(clip->getLeftPts(), -mDiff,  exclude);
    }
    else
    {
        if (mLeft)
        {
            if (mDiff > 0) // Reduce: Move clip begin point to the right
            {
                // Add empty clip in front of new clip: new clip is shorter than original clip and the frames should maintain their position.
                replace.push_front(boost::make_shared<model::EmptyClip>(mDiff));
                replacelink.push_front(boost::make_shared<model::EmptyClip>(mDiff));
            }
            else // (mDiff < 0) // Enlarge: Move clip begin point to the left
            {
                ASSERT(clip->getPrev()->isA<model::EmptyClip>());
                reduceSize(clip->getPrev(), 0, -mDiff);
                if (linked)
                {
                    ASSERT(linked->getPrev()->isA<model::EmptyClip>());
                    reduceSize(linked->getPrev(), 0, -mDiff);
                }
            }
        }
        else // !mLeft
        {
            if (mDiff < 0) // Reduce: Move clip end point to the left
            {
                // Add empty clip after new clip: new clip is shorter than original clip and the frames should maintain their position.
                replace.push_back(boost::make_shared<model::EmptyClip>(-mDiff));
                replacelink.push_back(boost::make_shared<model::EmptyClip>(-mDiff));
            }
            else // (mDiff > 0) // Enlarge: Move clip end point to the right
            {
                ASSERT(clip->getNext()->isA<model::EmptyClip>());
                reduceSize(clip->getNext(), mDiff, 0);
                if (linked)
                {
                    ASSERT(linked->getNext()->isA<model::EmptyClip>());
                    reduceSize(linked->getNext(), mDiff, 0);
                }
            }
        }
    }

    replaceClip(clip, replace);
    if (linked)
    {
        replaceClip(linked, replacelink);
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TrimClip& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mClip << '|' << obj.mDiff << '|' << obj.mLeft << '|' << obj.mShift;
    return os;
}

}}} // namespace