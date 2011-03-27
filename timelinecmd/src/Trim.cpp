#include "Trim.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "Timeline.h"
#include "Cursor.h"
#include "Track.h"
#include "Sequence.h"
#include "Zoom.h"
#include "EmptyClip.h"
#include "Clip.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Trim::Trim(gui::timeline::Timeline& timeline, model::ClipPtr clip, pts diff, bool left, bool shift)
    :   AClipEdit(timeline)
    ,   mClip(clip)
    ,   mDiff(diff)
    ,   mLeft(left)
    ,   mShift(shift)
{
    VAR_INFO(this)(mClip)(mDiff)(mLeft)(mShift);
    ASSERT(mDiff != 0); // Useless to add an action to the undo list, when there is no change
    if (mLeft)
    {
        mCommandName = _("Adjust clip begin point");
    }
    else
    {
        mCommandName = _("Adjust clip end point");
    }
}

Trim::~Trim()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void Trim::initialize()
{
    VAR_INFO(this);
    model::ClipPtr newclip;
    model::ClipPtr newlink;

    model::ClipPtr linked = mClip->getLink();

    newclip = make_cloned<model::Clip>(mClip);
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
        // NIY: What if the link is 'shifted' wrt original clip?
        ASSERT(mClip->getLeftPts() == linked->getLeftPts());
        ASSERT(mClip->getRightPts() == linked->getRightPts());

        newlink = make_cloned<model::Clip>(linked);
        if (mLeft)
        {
            newlink->adjustBegin(mDiff);
        }
        else
        {
            newlink->adjustEnd(mDiff);
        }
    }

    ReplacementMap linkmapper;
    model::Clips replace = boost::assign::list_of(newclip);
    model::Clips replacelink = boost::assign::list_of(newlink);

    // If the clip or its link is resized to 0 frames, then replace the original clip with nothing
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
        model::Tracks exclude = boost::assign::list_of(mClip->getTrack());
        if (linked)
        {
            exclude.push_back(linked->getTrack());
        }

        // Note that the state class is responsible for checking that there is enough space
        // in case of moving clips to the left.

        // \todo what if the linked clip is more to the left. Then that position should 
        // be used for shifting other tracks?
        ASSERT(mClip->getLeftPts() == linked->getLeftPts());
        ASSERT(mClip->getRightPts() == linked->getRightPts());
        // end todo

        shiftAllTracks(mClip->getLeftPts(), -mDiff,  exclude);
    }
    else
    {
        if (mLeft)
        {
            if (mDiff > 0) // Reduce: Move clip begin point to the right
            {
                // Add empty clip in front of new clip: new clip is shorter than original clip and the frames should maintain their position.
                replace.push_front(makeEmptyClip(mDiff));
                replacelink.push_front(makeEmptyClip(mDiff));
            }
            else // (mDiff < 0) // Enlarge: Move clip begin point to the left
            {
                removehitespace(mClip->getTrack()->getPreviousClip(mClip), -mDiff, &linkmapper);
                if (linked)
                {
                    removehitespace(linked->getTrack()->getPreviousClip(linked), -mDiff, &linkmapper);
                }
            }
        }
        else // !mLeft
        {
            if (mDiff < 0) // Reduce: Move clip end point to the left
            {
                // Add empty clip after new clip: new clip is shorter than original clip and the frames should maintain their position.
                replace.push_back(makeEmptyClip(-mDiff));
                replacelink.push_back(makeEmptyClip(-mDiff));
            }
            else // (mDiff > 0) // Enlarge: Move clip end point to the right
            {
                removehitespace(mClip->getTrack()->getNextClip(mClip), mDiff, &linkmapper);
                if (linked)
                {
                    removehitespace(linked->getTrack()->getNextClip(linked), mDiff, &linkmapper);
                }
            }

        }
    }

    replaceClip(mClip, replace, &linkmapper);
    if (linked)
    {
        replaceClip(mClip->getLink(), replacelink, &linkmapper);
    }
    replaceLinks(linkmapper);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Trim::removehitespace(model::ClipPtr emptyclip, pts toberemoved, ReplacementMap* conversionmap)
{
    ASSERT(emptyclip && emptyclip->isA<model::EmptyClip>() && emptyclip->getLength() >= toberemoved); // The area to be removed must be available
    replaceClip(emptyclip, makeEmptyClips(emptyclip->getLength() - toberemoved), conversionmap);
}

}}} // namespace