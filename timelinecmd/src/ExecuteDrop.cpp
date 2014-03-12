// Copyright 2013,2014 Eric Raijmakers.
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

#include "ExecuteDrop.h"

#include "Calculate.h"
#include "Clip.h"
#include "Drag_Shift.h"
#include "Logging.h"
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
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

Drop::Drop()
    : track()
    , position(0)
    , clips()
{
}

Drop::Drop(const Drop& other)
    : track(other.track)
    , position(other.position)
    , clips(other.clips)
{
}

Drop::~Drop()
{
}

std::ostream& operator<<(std::ostream& os, const Drop& obj)
{
    os << &obj << '|' << obj.track << '|' << obj.position << '|' << obj.clips;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ExecuteDrop::ExecuteDrop(const model::SequencePtr& sequence)
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

void ExecuteDrop::onDragStart(const Drags& drags, bool mIsInsideDrag)
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
        for ( model::IClipPtr clip : drags )
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
        for ( model::TransitionPtr transition : allTransitions )
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
        for ( model::TransitionPtr transition : unapplied )
        {
            // Since no new clips are made for dropping the clips (they're simply removed, keeping the undo
            // history small, by avoiding cloning constantly), clips that are linked to the clips replaced by
            // unapplying the transition must be replaced with clones also (otherwise these links had to be
            // linked both to the original AND the replacement clips which is impossible). Hence: true.
            unapplyTransition(transition, true);
        }

        // Now determine which clips are being dragged. This differs from the input list for the
        // following reasons:
        // - Unapply transitions causes the timeline to change
        //   * unapplied transitions do not have to be dragged
        //   * the clips changed by unapplying the transition do not have to be dragged (but their replacements do!)
        //   * clips linked to the clips adjacent to the transition are also replaced
        // - Transitions that are not selected, but their adjacent clips are, are also dragged automatically
        // Since the sequence was (possibly) changed, the initial lists consists of all clips that are
        // selected after applying those changes.
        UtilSet<model::IClipPtr>(mDrags).addElements(getSequence()->getSelectedClips());
        for ( model::TransitionPtr transition : allTransitions )
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
        for ( model::IClipPtr clip : getDrags() )
        {
            clip->setDragged(true);
        }
    }

    VAR_INFO(mDrags);
}

void ExecuteDrop::onDrop(const Drops& drops, const Shift& shift)
{
    VAR_INFO(this)(drops)(shift);
    mDrops = drops;
    mShift = shift;
    for ( model::IClipPtr clip : getDrags() )
    {
        clip->setDragged(false);
    }
}

void ExecuteDrop::onAbort()
{
    for ( model::IClipPtr clip : getDrags() )
    {
        clip->setDragged(false);
    }
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void ExecuteDrop::initialize()
{
    VAR_INFO(this);

    LOG_INFO << "STEP 1: Replace all drags with EmptyClips";
    for ( model::IClipPtr clip : mDrags )
    {
        // If ever this mechanism (replace clip by clip) is replaced, take into account that the
        // clips in mDrags are not 'in timeline order' in the set.
        VAR_INFO(clip);

        // Note that specifically no 'link replacement' is done. Keeping links intact is done
        // with the 'drops', not the 'drags'. Note that clips typically are part of both mDrags
        // AND mDrops. Replacing them first with empty clips here, and then later on with other
        // clips (when dropping, for instance) causes problems (unable to correctly map the links).
        replaceClip(clip,boost::assign::list_of(model::EmptyClip::replace(clip)),false);
    }

    if (mShift)
    {
        LOG_INFO << "STEP 2: Apply shift";
        for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(mShift->getPtsPosition());
            addClip(boost::make_shared<model::EmptyClip>(mShift->getPtsLength()), track, clip );
        }
    }
    else
    {
        LOG_INFO << "STEP 2: Apply shift (none)";
    }

    LOG_INFO << "STEP 3: Execute the drops";
    for ( Drop drop : mDrops )
    {
        ASSERT_MORE_THAN_EQUALS_ZERO(drop.position);
        ASSERT(drop.track);
        ASSERT_NONZERO(drop.clips.size());
        VAR_INFO(drop.position)(drop.track)(drop.clips);
        LOG_INFO << "Dropping:" << DUMP(drop.track) << DUMP(drop.clips);

        // Determine size and end pts of dropped clips
        pts droppedSize = model::calculate::combinedLength(drop.clips);
        pts dropEndPosition = drop.position + droppedSize;

        LOG_INFO << "STEP 3a: Splitting at beginning of drop position";

        // Ensure that the track has cuts at the begin and the end of the dropped clips
        split(drop.track, drop.position);

        LOG_INFO << "STEP 3b: Splitting at end of drop position";

        split(drop.track, dropEndPosition);

        // Determine the clips to be replaced.
        // Done AFTER the splitting above, since that requires clip addition/removal.
        // Furthermore, note that the split calls above ensure that any transitions at
        // the begin and end of the insertion are removed.
        AClipEdit::ClipsWithPosition remove = findClips(drop.track, drop.position, dropEndPosition);

        if (drop.position > drop.track->getLength())
        {
            LOG_INFO << "Step 3c: Drop is beyond track length. Adding empty clip before dropped clip,";
            // Drop is beyond track length. Add an empty clip to have it a at the desired position (instead of directly after last clip).
            ASSERT(!remove.second)(remove.second); // The position of the drop should be a null ptr, since the drop is at the end of the track
            drop.clips.push_front(boost::make_shared<model::EmptyClip>(drop.position - drop.track->getLength()));
        }
        else if (drop.position == drop.track->getLength())
        {
            LOG_INFO << "Step 3c: Drop is exactly at end of track. No extra action required.";
            // Drop is exactly at end of track. Nothing needs to be removed. Nothing needs to be added.
        }
        else
        {
            LOG_INFO << "Step 3c: Drop inside track. Removing required clips to make room.";
            // Drop 'inside' track.
            // Remove the clips that are 'under' the drop. Note that link replacing is required here. Consider the scenario in which
            // a audio-only clip is dropped onto the beginning of a audio clip that is linked to a video clip. First, the 'dropped upon' audio clip
            // is split into two parts (thus, the replacements of that clip are two clips, called 'left' and 'right'). Then, 'left' is removed again.
            // During link replacing the original audio clip must be linked to 'right'. That requires a replacement mapping of 'left' to '' (empty list).
            removeClips(remove.first);
        }

        // Finally, insert the dropped clips.
        LOG_INFO << "STEP 3d: Dropping the clips";
        addClips(drop.clips, drop.track, remove.second);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

const Drags& ExecuteDrop::getDrags() const
{
    return mDrags;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ExecuteDrop& obj)
{
    os << static_cast<const AClipEdit&>(obj) << '|' << '|' << obj.mDrags << '|' << obj.mShift;
    return os;
}
}}} // namespace