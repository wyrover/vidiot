#include "ExecuteDrop.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Clip.h"
#include "EmptyClip.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilSet.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace gui { namespace timeline { namespace command {

// static
const pts ExecuteDrop::sNoShift = -1;

ExecuteDrop::ExecuteDrop(model::SequencePtr sequence, std::set<model::IClipPtr> drags, Drops drops, pts shiftPosition, pts shiftSize)
:   AClipEdit(sequence)
,   mTransitions()
,   mDrags(drags)
,   mDrops(drops)
,   mShiftPosition(shiftPosition)
,   mShiftSize(shiftSize)
{
    VAR_INFO(this)(drags)(drops)(shiftPosition)(shiftSize);
    mCommandName = _("Move clips");
}

ExecuteDrop::~ExecuteDrop()
{
}

void ExecuteDrop::initialize()
{
    VAR_INFO(this);

    ReplacementMap linkmapper;

    LOG_DEBUG << "STEP 1: Remove all transitions for which the transitioned clips are 'torn apart'";
    BOOST_FOREACH( model::IClipPtr clip, mDrags )
    {
        model::TrackPtr track = clip->getTrack();
        model::TransitionPtr prevTransition = boost::dynamic_pointer_cast<model::Transition>(clip->getPrev());
        if (prevTransition && transitionMustBeRemovedOnDrop(prevTransition))
        {
            removeTransition(prevTransition, linkmapper);
        }
        model::TransitionPtr nextTransition = boost::dynamic_pointer_cast<model::Transition>(clip->getNext());
        if (nextTransition && transitionMustBeRemovedOnDrop(nextTransition))
        {
            removeTransition(nextTransition, linkmapper);
        }
    }

    LOG_DEBUG << "STEP 2: Replace all drags with EmptyClips";
    BOOST_FOREACH( model::IClipPtr clip, mDrags )
    {
        // If ever this mechanism (replace clip by clip) is replaced, take into account that the
        // clips in mDrags are not 'in timeline order' in the set.
        replaceClip(clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getLength())));
    }

    if (mShiftPosition >= 0)
    {
        LOG_DEBUG << "STEP 3: Apply shift";
        BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(mShiftPosition);
            newMove(track, clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(mShiftSize)));
        }
    }
    else
    {
        LOG_DEBUG << "STEP 3: Apply shift (none)";
    }

    LOG_DEBUG << "STEP 4: Execute the drops AND fill replacement map";
    BOOST_FOREACH( Drop drop, mDrops )
    {
        ASSERT_MORE_THAN_EQUALS_ZERO(drop.position);
        ASSERT(drop.track);
        ASSERT_NONZERO(drop.clips.size());
        VAR_DEBUG(drop.position)(drop.track)(drop.clips);

        // Determine size and end pts of dropped clips
        pts droppedSize = model::Track::getCombinedLength(drop.clips);
        pts dropEndPosition = drop.position + droppedSize;

        // Ensure that the track has cuts at the begin and the end of the dropped clips
        split(drop.track, drop.position,   &linkmapper);
        split(drop.track, dropEndPosition, &linkmapper);

        // Determine the clips to be replaced.
        // Done AFTER the splitting above, since that requires clip addition/removal.
        // Furthermore, note that the split calls above ensure that any transitions at
        // the begin and end of the insertion are removed.
        AClipEdit::ClipsWithPosition remove = findClips(drop.track, drop.position, dropEndPosition);

        if (drop.position > drop.track->getLength())
        {
            // Drop is beyond track length. Add an empty clip to have it a at the desired position (instead of directly after last clip).
            ASSERT(!remove.second)(remove.second); // The position of the drop should be a null ptr, since the drop is at the end of the track
            newMove(drop.track, remove.second, boost::assign::list_of(boost::make_shared<model::EmptyClip>(drop.position - drop.track->getLength())) );
        }

        //      ================ ADD ===============  =============== REMOVE ================
        newMove(drop.track, remove.second, drop.clips, drop.track, remove.second, remove.first);
    }

    LOG_DEBUG << "STEP 5: Ensure that links are maintained.";
    replaceLinks(linkmapper);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ExecuteDrop::Drop& obj )
{
    os << &obj << '|' << obj.track << '|' << obj.position << '|' << obj.clips;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool ExecuteDrop::transitionMustBeRemovedOnDrop(model::TransitionPtr transition) const
{
    bool adjacentClipDragged = false;
    bool adjacentClipMissing = false;
    if (transition->getLeft() > 0)
    {
        model::IClipPtr prev = transition->getPrev();
        ASSERT(prev);
        if (mDrags.find(prev) != mDrags.end())
        {
            adjacentClipDragged = true;
        }
        else
        {
            adjacentClipMissing = true;
        }
    }
    if (transition->getRight() > 0)
    {
        model::IClipPtr next = transition->getNext();
        ASSERT(next);
        if (mDrags.find(next) != mDrags.end())
        {
            adjacentClipDragged = true;
        }
        else
        {
            adjacentClipMissing = true;
        }
    }
    return adjacentClipDragged && adjacentClipMissing;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ExecuteDrop& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mTransitions << '|' << obj.mDrags << '|' << obj.mShiftPosition << '|' << obj.mShiftSize;
    return os;
}

}}} // namespace
