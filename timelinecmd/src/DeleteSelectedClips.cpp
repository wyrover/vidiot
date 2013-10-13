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
    LOG_DEBUG << "STEP 1: Determine which transitions must be unapplied.";
    std::set<model::TransitionPtr> transitionsToBeUnapplied;
    for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
    {
        for ( model::IClipPtr clip : track->getClips() )
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
                    if (clip->getInTransition())
                    {
                        transitionsToBeUnapplied.insert(clip->getInTransition());
                    }
                    if (clip->getOutTransition())
                    {
                        transitionsToBeUnapplied.insert(clip->getOutTransition());
                    }
                }
            }
        }
    }

    LOG_DEBUG << "STEP 2: Unapply transitions.";
    for ( model::TransitionPtr transition : transitionsToBeUnapplied )
    {
        unapplyTransition(transition);
    }

    LOG_DEBUG << "STEP 3: Determine the clips to be removed.";
    std::list<model::IClipPtr> clipsToBeRemoved;
    {
        for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
        {
            for ( model::IClipPtr clip : track->getClips() )
            {
                if (clip->getSelected())
                {
                    ASSERT(!clip->isA<model::Transition>()); // Should have been unapplied already
                    ASSERT(!clip->getInTransition()); // Should have been unapplied already
                    ASSERT(!clip->getOutTransition()); // Should have been unapplied already
                    clipsToBeRemoved.push_back(clip);
                }
            }
        }
    }

    LOG_DEBUG << "STEP 4: Delete clips.";
    if (mShift)
    {
        animatedDeleteAndTrim(clipsToBeRemoved);
    }
    else
    {
        getTimeline().beginTransaction();
        for ( model::IClipPtr clip : clipsToBeRemoved )
        {
            replaceClip(clip,boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getLength())));
        }
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
    for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
    {
        for ( model::IClipPtr clip : track->getClips() )
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
    for ( model::IClipPtr clip : mSelected )
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