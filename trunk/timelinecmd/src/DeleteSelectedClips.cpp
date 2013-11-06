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
    PtsIntervals deletionRegion; // Region covered by the deleted clips
    std::map<model::TrackPtr, PtsIntervals> deletionRegionTrack;
    {
        for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
        {
            deletionRegionTrack[track] = PtsIntervals();
            for ( model::IClipPtr clip : track->getClips() )
            {
                if (clip->getSelected())
                {
                    ASSERT(!clip->isA<model::Transition>()); // Should have been unapplied already
                    ASSERT(!clip->getInTransition()); // Should have been unapplied already
                    ASSERT(!clip->getOutTransition()); // Should have been unapplied already
                    clipsToBeRemoved.push_back(clip);
                    deletionRegion += PtsInterval(clip->getLeftPts(), clip->getRightPts());
                    deletionRegionTrack[track] += PtsInterval(clip->getLeftPts(), clip->getRightPts());
                }
            }
        }
    }

    LOG_DEBUG << "STEP 4: For shift trimming determine if all tracks can/will be shifted properly.";
    bool doShift = mShift;
    if (doShift)
    {
        std::map<model::IClipPtr, PtsInterval> emptyClipsToBeSplit;
        for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
        {
            if (!doShift) { break; }

            if (deletionRegionTrack[track] != deletionRegion)
            {
                ASSERT(boost::icl::contains(deletionRegion, deletionRegionTrack[track]))(track)(deletionRegion)(deletionRegionTrack[track]);

                // If, and only if, the part of the track that is not being deleted consists of empty
                // clips (and nothing but empty clips), then it can be trimmed away nicely.

                PtsIntervals remainingRegionInTrack = deletionRegion;
                remainingRegionInTrack -= deletionRegionTrack[track];

                for ( PtsInterval interval : remainingRegionInTrack )
                {
                   model::IClipPtr clip = track->getClip(interval.lower());
                   if (clip->isA<model::EmptyClip>() &&
                       clip->getLeftPts() <= interval.lower() &&
                       clip->getRightPts() >= interval.upper())
                   {
                       ASSERT_MAP_CONTAINS_NOT(emptyClipsToBeSplit, clip);
                       emptyClipsToBeSplit[clip] = interval;
                   }
                   else
                   {
                       doShift = false;
                       break;
                   }
                }
            }
        }

        if (doShift)
        {
            // Shifting is still possible. Now split empty clips where needed,
            // and add the resulting empty clips to the list of clips to be removed.
            for ( std::map<model::IClipPtr, PtsInterval>::value_type clipAndInterval : emptyClipsToBeSplit )
            {
                model::IClipPtr clip = clipAndInterval.first;
                ASSERT(clip->isA<model::EmptyClip>())(clip);
                PtsInterval interval = clipAndInterval.second;
                model::TrackPtr track = clip->getTrack();
                split(track, interval.lower());
                split(track, interval.upper());
                model::IClipPtr toBeRemoved = track->getClip(interval.lower());
                ASSERT_EQUALS(toBeRemoved->getLeftPts(), interval.lower());
                ASSERT_EQUALS(toBeRemoved->getRightPts(), interval.upper());
                clipsToBeRemoved.push_back(toBeRemoved);
            }
        }
        else
        {
            gui::StatusBar::get().timedInfoText(_("Could not shift clips. Part of the region is still in use."));
        }
    }

    LOG_DEBUG << "STEP 5: Delete clips.";
    if (doShift)
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const DeleteSelectedClips& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mShift;
    return os;
}

}}} // namespace