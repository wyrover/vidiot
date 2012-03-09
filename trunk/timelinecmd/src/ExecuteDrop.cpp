#include "ExecuteDrop.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Calculate.h"
#include "Clip.h"
#include "EmptyClip.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSet.h"

namespace gui { namespace timeline { namespace command {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ExecuteDrop::ExecuteDrop(model::SequencePtr sequence)
    :   AClipEdit(sequence)
    ,   mDrags()
    ,   mDrops()
    ,   mShift()
{
    VAR_INFO(this);
    mCommandName = _("Move clips");
}

ExecuteDrop::~ExecuteDrop()
{
}

void ExecuteDrop::onDrag(const Drags& drags, bool mIsInsideDrag)
{
    VAR_INFO(this)(drags)(mIsInsideDrag);

    if (!mIsInsideDrag)
    {
        mDrags = drags;
    }
    else
    {
        // Determine all impacted transitions:
        // - all transitions that are selected, AND
        // - all transitions for which at least one of the adjacent (relevant) clips is selected.
        std::set<model::TransitionPtr> allTransitions;
        BOOST_FOREACH( model::IClipPtr clip, drags )
        {
            model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
            if (transition)
            {
                allTransitions.insert(transition);
            }
            else
            {
                model::TransitionPtr inTransition = clip->getInTransition();
                if (inTransition)
                {
                    allTransitions.insert(inTransition);
                }
                model::TransitionPtr outTransition = clip->getOutTransition();
                if (outTransition)
                {
                    allTransitions.insert(outTransition);
                }
            }
        }

        // Now determine all transitions that must be unapplied. Any transition that is an in-out-transition,
        // for which only one of the two adjacent clips is dragged, must be unapplied. The unapply is done
        // immediately, for the following purposes:
        // - immediate feedback
        // - no hassle later on with having to take into account that the timeline changes because of this
        std::set<model::TransitionPtr> unapplied;
        BOOST_FOREACH( model::TransitionPtr transition, allTransitions )
        {
            if (transition->getLeft() > 0 && transition->getRight() > 0)
            {
                ASSERT(transition->getPrev() && transition->getNext())(transition);
                if (!transition->getPrev()->getSelected() || !transition->getNext()->getSelected())
                {
                    unapplied.insert(transition);
                }
            }
        }
        BOOST_FOREACH( model::TransitionPtr transition, unapplied )
        {
            unapplyTransition(transition);
        }

        // Now determine which clips are being dragged. This differs from the input list for the
        // following reasons:
        // - Unapply transitions causes the timeline to change (unapplied transitions and their adjacent clips do not need to be dragged anymore)
        // - Transitions that are not selected, but their adjacent clips are, are also dragged automatically
        // Since the sequence was (possibly) changed, the initial lists consists of all clips that are
        // selected after applying those changes.
        UtilSet<model::IClipPtr>(mDrags).addElements(getSequence()->getSelectedClips());
        BOOST_FOREACH( model::TransitionPtr transition, allTransitions )
        {
            // The transition is known to be 'impacted'. Therefore, it is already selected, or one of it's
            // adjacent clips is selected.
            // For in-only, or out-only transitions, this means that the transition MUST be selected also.
            // For in-out-transitions, if not unapplied, this means that both adjacent clips are selected
            // and thus the transition must be dragged also.
            if (unapplied.find(transition) != unapplied.end()) continue;

            ASSERT((transition->getRight() > 0) || (transition->getPrev() && transition->getPrev()->getSelected()));
            ASSERT((transition->getLeft()  > 0) || (transition->getNext() && transition->getNext()->getSelected()));

            UtilSet<model::IClipPtr>(mDrags).addElement(transition); // This insertion is not 'in order'. If the transition already was part, then the use of std::set ensures that it's only present once.
        }
        BOOST_FOREACH( model::IClipPtr clip, getDrags() )
        {
            clip->setDragged(true);
        }
    }

    VAR_DEBUG(mDrags);
}

void ExecuteDrop::onDrop(Drops drops, Shift shift)
{
    VAR_INFO(this)(drops)(shift);
    mDrops = drops;
    mShift = shift;
    BOOST_FOREACH( model::IClipPtr clip, getDrags() )
    {
        clip->setDragged(false);
    }
}

void ExecuteDrop::onAbort()
{
    BOOST_FOREACH( model::IClipPtr clip, getDrags() )
    {
        clip->setDragged(false);
    }
    Undo();
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void ExecuteDrop::initialize()
{
    VAR_INFO(this);

    LOG_DEBUG << "STEP 1: Replace all drags with EmptyClips";
    BOOST_FOREACH( model::IClipPtr clip, mDrags )
    {
        // If ever this mechanism (replace clip by clip) is replaced, take into account that the
        // clips in mDrags are not 'in timeline order' in the set.
        VAR_DEBUG(clip);
        replaceClip(clip, boost::assign::list_of(model::EmptyClip::replace(clip)));
    }

    if (mShift)
    {
        LOG_DEBUG << "STEP 2: Apply shift";
        BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(mShift->mPosition);

            addClip(boost::make_shared<model::EmptyClip>(mShift->mLength), track, clip );
        }
    }
    else
    {
        LOG_DEBUG << "STEP 2: Apply shift (none)";
    }

    LOG_DEBUG << "STEP 3: Execute the drops";
    BOOST_FOREACH( Drop drop, mDrops )
    {
        ASSERT_MORE_THAN_EQUALS_ZERO(drop.position);
        ASSERT(drop.track);
        ASSERT_NONZERO(drop.clips.size());
        VAR_DEBUG(drop.position)(drop.track)(drop.clips);

        // Determine size and end pts of dropped clips
        pts droppedSize = model::calculate::combinedLength(drop.clips);
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

        // Instead of dropping the clips themselves, clones of these clips are dropped. This is done to avoid recursion:
        // - Clip A is dragged, hence replaced with an emptyclip
        // - Clip A is dropped onto it's original position (at least partially over that empty clip)
        // Then, without this cloning, A->Empty->A in terms of replacements. That would cause stack overflows
        // (indefinite recursion) when expanding the replacements.
        replaceClips(remove.first, make_cloned(drop.clips));
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

const ExecuteDrop::Drags& ExecuteDrop::getDrags() const
{
    return mDrags;
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ExecuteDrop& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << '|' << obj.mDrags << '|' << obj.mShift;
    return os;
}
}}} // namespace