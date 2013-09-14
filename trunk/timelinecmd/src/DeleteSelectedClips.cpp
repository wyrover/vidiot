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

#include "DeleteSelectedClips.h"

#include "Clip.h"
#include "EmptyClip.h"
#include "Keyboard.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSet.h"

namespace gui { namespace timeline { namespace command {

DeleteSelectedClips::DeleteSelectedClips(model::SequencePtr sequence, bool shift)
    :   AClipEdit(sequence)
    ,   mShift(shift || getTimeline().getKeyboard().getShiftDown())
{
    VAR_INFO(this)(mShift);
    mCommandName = _("Delete selected clips");
}

DeleteSelectedClips::~DeleteSelectedClips()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void DeleteSelectedClips::initialize()
{
    LOG_DEBUG << "STEP 1: Determine the clips to be removed, the transitions to be removed, and the transitions to be unapplied";
    std::set<model::TransitionPtr> transitionsToBeRemoved;
    std::set<model::TransitionPtr> transitionsToBeUnapplied;
    std::set<model::IClipPtr> clipsToBeRemoved;

    BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (clip->getSelected())
            {
                model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
                if (transition)
                {
                    transitionsToBeUnapplied.insert(transition);
                }
                else
                {
                    auto add = [&transitionsToBeRemoved] (model::TransitionPtr transition)
                    {
                        if (transition)
                        {
                            transitionsToBeRemoved.insert(transition);
                        }
                    };
                    add(clip->getInTransition());
                    add(clip->getOutTransition());
                    clipsToBeRemoved.insert(clip);
                }
            }
        }
    }

    // Remove/unapply transitions. Is done in separate steps to simplify the various possible options (transitions with and without left and right clips)
    // and to avoid problems with removing the left clip first, without the transition or removing the transition before the right clip
    BOOST_FOREACH( model::TransitionPtr transition, transitionsToBeRemoved )
    {
        // Transitions that are deleted (one of their clips is deleted also) do not have to be unapplied.
        transitionsToBeUnapplied.erase(transition); // If it is part of the set it is erased. Nothing is changed if it's not part of the set.
        clipsToBeRemoved.insert(boost::static_pointer_cast<model::IClip>(transition));
    }

    if (mShift)
    {
        LOG_DEBUG << "STEP 2a: Show animation.";
        // Animate the shifting of clips onto the empty space
        model::IClips emptyareas;
        BOOST_FOREACH( model::IClipPtr clip, clipsToBeRemoved )
        {
            model::IClipPtr emptyness = boost::make_shared<model::EmptyClip>(clip->getLength());
            emptyareas.push_back(emptyness);
            replaceClip(clip,boost::assign::list_of(emptyness));
        }
        animatedTrimEmpty(emptyareas);

        getTimeline().beginTransaction();
        Revert();
        LOG_DEBUG << "STEP 2b: Make the actual change.";
        BOOST_FOREACH( model::IClipPtr clip, clipsToBeRemoved )
        {
            removeClip(clip);
        }
    }
    else
    {
        LOG_DEBUG << "STEP 2: Make the actual change.";
        getTimeline().beginTransaction();
        BOOST_FOREACH( model::IClipPtr clip, clipsToBeRemoved )
        {
            replaceClip(clip,boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getLength())));
        }
    }

    LOG_DEBUG << "STEP 3: Unapply deleted transitions (for which no adjacent clip was selected).";
    BOOST_FOREACH( model::TransitionPtr transition, transitionsToBeUnapplied )
    {
        unapplyTransition(transition);
    }
    getTimeline().endTransaction();
}

void DeleteSelectedClips::doExtraBefore()
{
    storeSelection();
}

void DeleteSelectedClips::undoExtraAfter()
{
    restoreSelection();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DeleteSelectedClips::storeSelection()
{
    BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (clip->getSelected())
            {
                mSelected.push_back(clip);
            }
        }
    }
}

void DeleteSelectedClips::restoreSelection()
{
    getTimeline().getSelection().unselectAll();
    BOOST_FOREACH( model::IClipPtr clip, mSelected )
    {
        clip->setSelected(true);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const DeleteSelectedClips& obj )
{
    os << static_cast<const AClipEdit&>(obj);
    return os;
}

}}} // namespace