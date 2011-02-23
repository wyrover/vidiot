#include "TrimBegin.h"

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

TrimBegin::TrimBegin(gui::timeline::Timeline& timeline, model::ClipPtr clip, pts diff, bool shift)
    :   AClipEdit(timeline)
    ,   mClip(clip)
    ,   mDiff(diff)
    ,   mShift(shift)
{
    VAR_INFO(this)(mClip)(mDiff)(mShift);
    ASSERT(mDiff != 0); // Useless to add an action to the undo list, when there is no change
    mCommandName = _("Adjust clip begin point");
}

TrimBegin::~TrimBegin()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimBegin::initialize()
{
    VAR_INFO(this);
    model::ClipPtr newclip;
    model::ClipPtr newlink;

    model::ClipPtr linked = mClip->getLink();

    newclip = make_cloned<model::Clip>(mClip);
    newclip->adjustBegin(mDiff);

    if (linked)
    {
        // NIY: What if the link is 'shifted' wrt original clip?
        ASSERT(mClip->getLeftPts() == linked->getLeftPts());

        newlink = make_cloned<model::Clip>(linked);
        newlink->adjustBegin(mDiff);
    }

    ReplacementMap linkmapper;
    model::Clips replace = boost::assign::list_of(newclip);
    model::Clips replacelink = boost::assign::list_of(newlink);

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
        // end todo

        shiftAllTracks(mClip->getLeftPts(), -mDiff,  exclude);
    }
    else
    {
        if (mDiff > 0) // Reduce: Move clip begin point to the right
        {
            // Add empty clip in front of new clip: new clip is shorter than original clip and the frames should maintain their position.
            replace.push_front(makeEmptyClip(mDiff));
            replacelink.push_front(makeEmptyClip(mDiff));
        }
        else // (mDiff < 0) // Enlarge: Move clip begin point to the left
        {
            // Remove whitespace in front of original clip: new clip is longer than original clip and the frames should maintain their position
            model::ClipPtr emptyclip = mClip->getTrack()->getPreviousClip(mClip);
            ASSERT(emptyclip && emptyclip->isA<model::EmptyClip>() && emptyclip->getNumberOfFrames() >= -mDiff); // The enlarged clip must fit
            replaceClip(emptyclip, makeEmptyClips(emptyclip->getNumberOfFrames() + mDiff), &linkmapper); // Replace the original empty clip

            if (linked)
            {
                model::ClipPtr emptylink = linked->getTrack()->getPreviousClip(linked);
                ASSERT(emptylink && emptylink->isA<model::EmptyClip>() && emptylink->getNumberOfFrames() >= -mDiff); // The enlarged linked clip must fit
                replaceClip(emptylink, makeEmptyClips(emptylink->getNumberOfFrames() + mDiff), &linkmapper); // Replace the original empty clip
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

}}} // namespace
