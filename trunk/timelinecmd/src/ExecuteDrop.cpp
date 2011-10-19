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

    LOG_DEBUG << "STEP 1: Determine the transitions for which the transitioned clips are 'torn apart'";
    // todo dit naderhand doen: alle clips doorlopen op 'invalid' transitions

    std::set<model::TransitionPtr> transitionsToBeUnapplied;
    BOOST_FOREACH( model::IClipPtr clip, mDrags )
    {
        model::TrackPtr track = clip->getTrack();
        model::TransitionPtr prevTransition = boost::dynamic_pointer_cast<model::Transition>(clip->getPrev());
        if (prevTransition && transitionMustBeUnapplied(prevTransition))
        {
            VAR_DEBUG(prevTransition);
            transitionsToBeUnapplied.insert(prevTransition);
        }
        model::TransitionPtr nextTransition = boost::dynamic_pointer_cast<model::Transition>(clip->getNext());
        if (nextTransition && transitionMustBeUnapplied(nextTransition))
        {
            transitionsToBeUnapplied.insert(nextTransition);
            VAR_DEBUG(nextTransition);
        }
    }

    LOG_DEBUG << "STEP 2: Replace all drags with EmptyClips";
    BOOST_FOREACH( model::IClipPtr clip, mDrags )
    {
        // If ever this mechanism (replace clip by clip) is replaced, take into account that the
        // clips in mDrags are not 'in timeline order' in the set.
        VAR_DEBUG(clip);
        replaceClip(clip, boost::assign::list_of(model::EmptyClip::replace(clip)));
    }

    if (mShiftPosition >= 0)
    {
        LOG_DEBUG << "STEP 3: Apply shift";
        BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(mShiftPosition);

            addClip(boost::make_shared<model::EmptyClip>(mShiftSize), track, clip );
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
        split(drop.track, drop.position);
        split(drop.track, dropEndPosition);

        // Determine the clips to be replaced.
        // Done AFTER the splitting above, since that requires clip addition/removal.
        // Furthermore, note that the split calls above ensure that any transitions at
        // the begin and end of the insertion are removed.
        AClipEdit::ClipsWithPosition remove = findClips(drop.track, drop.position, dropEndPosition);

        if (drop.position > drop.track->getLength())
        {
            // Drop is beyond track length. Add an empty clip to have it a at the desired position (instead of directly after last clip).
            ASSERT(!remove.second)(remove.second); // The position of the drop should be a null ptr, since the drop is at the end of the track
            addClip(boost::make_shared<model::EmptyClip>(drop.position - drop.track->getLength()), drop.track, remove.second);
        }

        replaceClips(remove.first, drop.clips);
    }

    LOG_DEBUG << "STEP 5: Unapply 'torn apart' transitions that have not yet been removed";
    BOOST_FOREACH( model::TransitionPtr transition, transitionsToBeUnapplied )
    {
        if (!hasBeenReplaced(transition))
        {
            unapplyTransition(transition);
        }
    }
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

bool ExecuteDrop::transitionMustBeUnapplied(model::TransitionPtr transition) const
{
    bool adjacentClipDragged = false;
    bool adjacentClipNotDragged = false;
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
            adjacentClipNotDragged = true;
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
            adjacentClipNotDragged = true;
        }
    }
    return adjacentClipDragged && adjacentClipNotDragged;
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
