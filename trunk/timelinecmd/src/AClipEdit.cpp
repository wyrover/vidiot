// Copyright 2013-2015 Eric Raijmakers.
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

#include "AClipEdit.h"

#include "AudioClip.h"
#include "Clip.h"
#include "Config.h"
#include "Cursor.h"
#include "EmptyClip.h"
#include "EmptyClip.h"
#include "LinkReplacementMap.h"
#include "Logging.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackEvent.h"
#include "Transition.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSet.h"
#include "UtilVector.h"
#include "VideoClip.h"
#include "VideoTransition.h"

namespace gui { namespace timeline { namespace command {

    //////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AClipEdit::AClipEdit(const model::SequencePtr& sequence)
    : ATimelineCommand(sequence)
    , mParams()
    , mParamsUndo()
    , mReplacements(boost::make_shared<LinkReplacementMap>())
    , mInitialized(false)
{
    VAR_INFO(this);
}

AClipEdit::~AClipEdit()
{
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool AClipEdit::Do()
{
    VAR_INFO(*this)(mInitialized);

    doExtraBefore();

    if (!mInitialized)
    {
        initialize();

        mergeConsecutiveEmptyClips();

        avoidDanglingLinks();

        mReplacements->replace();
        if (Config::ReadBool(Config::sPathTestCxxMode))
        {
            // Only in test mode: verify all links.
            for (model::TrackPtr track : getSequence()->getTracks())
            {
                for (model::IClipPtr clip : track->getClips())
                {
                    if (clip->getLink())
                    {
                        if (clip != clip->getLink()->getLink())
                        {
                            LOG_ERROR << dump(getSequence());
                        }
                        ASSERT_EQUALS(clip, clip->getLink()->getLink());
                    }
                }
            }
        }

        removeEmptyClipsAtEndOfTracks();

        mInitialized = true;

        // No longer required (avoid extra memory use):
        mReplacements.reset();
    }
    else
    {
        ASSERT_NONZERO(mParams.size());
        for ( model::MoveParameterPtr move : mParams )
        {
            doMove(move);
        }
    }

    doExtraAfter();

    getTimeline().getSelection().updateOnEdit();

    if (!mParams.empty())
    {
        // Only if actual changes were done, this call ensures resetting of all 'iteration' variables.
        // Note that the if was added to avoid resetting the cursor (red line) in case of changing the length
        // of a clip via 'clip details view'. The Trim command used there did not do any changes in its
        // initialize method, but still (without the if) the modelChanged() was called. That caused the cursor
        // position to be changed again.
        //
        // Scenario that went wrong (without the if):
        // - Position cursor on clip 1
        // - Click clip 4
        // - Change a parameter of the clip (for instance, Y position)
        // - DetailsClip makes a new command: a combination of trimming (possible with offset 0) and transform clip.
        // - The 'previewed' frame of the modification should be a frame of clip 4;
        //   therefore DetailsClip.cpp repositions the cursor in this scenario.
        // However, the modelChanged() call here would cause an additional cursor repositioning.
        getTimeline().modelChanged();
    }

    if (Config::ReadBool(Config::sPathDebugLogSequenceOnEdit))
    {
        LOG_INFO << model::dump(getSequence(),1);
    }

    return true;
}

bool AClipEdit::Undo()
{
    VAR_INFO(*this)(mParamsUndo.size());
    //NOT: ASSERT_NONZERO(mParamsUndo.size()); - Due to the use in 'Revert()'

    undoExtraBefore();

    for ( model::MoveParameterPtr move : mParamsUndo )
    {
        doMove(move);
    }

    undoExtraAfter();

    getTimeline().getSelection().updateOnEdit();

    if (!mParamsUndo.empty())
    {
        getTimeline().modelChanged();
    }

    if (Config::ReadBool(Config::sPathDebugLogSequenceOnEdit))
    {
        LOG_INFO << model::dump(getSequence(),1);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void AClipEdit::doExtraBefore()
{
}

void AClipEdit::doExtraAfter()
{
}

void AClipEdit::undoExtraBefore()
{
}

void AClipEdit::undoExtraAfter()
{
}

void AClipEdit::initialize()
{
}

bool AClipEdit::isInitialized()
{
    return mInitialized;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS FOR SUBCLASSES
//////////////////////////////////////////////////////////////////////////

void AClipEdit::Revert()
{
    VAR_INFO(this);
    Undo();
    mParams.clear();
    mParamsUndo.clear();
    mReplacements = boost::make_shared<LinkReplacementMap>();
    getTimeline().getSelection().updateOnEdit();
    mInitialized = false;
}

void AClipEdit::split(const model::TrackPtr& track, pts position)
{
    model::IClipPtr clip = track->getClip(position);

    // Step 1: Remove any transitions directly adjacent to the split position.
    //         That includes transitions that have getLeftPts() == position
    //         AND transitions that have getRightPts() == position.
    // Note: track->getClip(position) returns the first clip that generates frames
    //       at 'position'. Due to the structuring of transitions it is possible
    //       that a transition that should be removed is not returned by this call:
    //       (1) A transition that touches directly with its right side.
    //           In this case, the clip after the transition may be returned.
    //       (2) A transition that touches directly with its right side, but its next
    //           clip has length 0 - thus, is only used to provide frames for the transition.
    //           In this case, the clip AFTER the 0-length clip is returned by 'getClip()'
    if (clip)
    {
        // If there already is a cut at the given position (leftpts == position) then there's no need to cut again.
        // One exception: if the cut is the 'edge' of a transition, the transition must be unapplied (particularly
        // required for drag and drop scenarios).
        // Doing the check at the beginning makes the handling of transitions (and the possible
        // 'unapplying of them' a bit simpler in the code below (no more checks for 'if already cut' required.

        model::TransitionPtr transition;
        ASSERT_LESS_THAN_EQUALS(clip->getLeftPts(),position);
        if (clip->getLeftPts() < position)
        {
            transition = boost::dynamic_pointer_cast<model::Transition>(clip);
        }
        else if (clip->getLeftPts() == position)
        {
            transition = boost::dynamic_pointer_cast<model::Transition>(clip);
            if (transition)
            {
                if (!transition->getLeft()) // Split is done on the left end of a transition.
                {
                    transition.reset();  // In only transition. Ignore.
                }
            }
            else
            {
                // This extra code is required because pts intervals are exclusive on the right edge
                // (thus "[left,right)"), but we want the transition to be removed here anyway.

                if (clip->getInTransition())
                {
                    // (1) clip is part of transition. The split is at the right edge of this transition. Remove it.
                    transition = clip->getInTransition(); // May be a 0-ptr
                }
                else if (clip->getPrev() && clip->getPrev()->getLength() == 0)
                {
                    // (2) Previous clip is of length 0 and (consequently) part of a transition.
                    // Thus, this transition is adjacent to 'position' also. Remove it.
                    transition = clip->getPrev()->getInTransition(); // May be 0-ptr
                }

                if (transition) // Split is done on the right end of a transition.
                {
                    if (!transition->getRight()) // Out only transition. Ignore.
                    {
                        transition.reset();
                    }
                }
            }
        }
        if (transition)
        {
            ASSERT_LESS_THAN_EQUALS(transition->getLeftPts(),position);
            unapplyTransition(transition,true);
            clip = track->getClip(position); // Must be done again, since unapplyTransition changes the track.
        }
    }

    // Step 2: split clip at given position.
    if (clip) // If there is a clip at the given position, it might need to be split
    {
        ASSERT(clip->isA<model::IClip>())(clip);
        position -= clip->getLeftPts();
        if (position != 0)
        {
            ASSERT(!clip->isA<model::Transition>())(clip);
            // If there is already a cut at the given position, nothing is changed.
            // Note that unapplying a transition creates a new cut, therefore this
            // check must be done again AFTER unapplyTransition has been applied.
            ASSERT_LESS_THAN(position,clip->getLength());
            model::IClipPtr left = make_cloned<model::IClip>(clip);
            model::IClipPtr right = make_cloned<model::IClip>(clip);
            left->adjustEnd(position - clip->getLength());
            right->adjustBegin(position);
            model::IClips replacements = { left, right };
            replaceClip(clip, replacements);
        }
    }
    // else: no clip: nothing to split
}

void AClipEdit::replaceClip(const model::IClipPtr& original, const model::IClips& replacements, bool maintainlinks)
{
    model::TrackPtr track = original->getTrack();
    ASSERT(track);
    model::IClipPtr position = original->getNext();
    model::IClips originallist = { original };

    if (maintainlinks)
    {
        mReplacements->add(original,replacements);
    }

    //      ============= ADD ===========  ========== REMOVE ===========
    newMove(track, position, replacements, track, position, originallist);
}

void AClipEdit::addClip(const model::IClipPtr& clip, const model::TrackPtr& track, const model::IClipPtr& position)
{
    ASSERT(!clip->getLink())(clip);
    addClips({ clip }, track, position);
}

void AClipEdit::addClips(const model::IClips& clips, const model::TrackPtr& track, const model::IClipPtr& position)
{
    newMove(track, position, clips );
}

void AClipEdit::removeClip(const model::IClipPtr& original)
{
    replaceClip(original, model::IClips());
}

void AClipEdit::removeClips(const model::IClips& originals)
{
    model::TrackPtr track = originals.front()->getTrack();
    ASSERT(track);
    model::IClipPtr position = originals.back()->getNext();

    for ( model::IClipPtr original : originals )
    {
        mReplacements->add(original, model::IClips());
    }

    //      ============== ADD =============  ======== REMOVE =========
    newMove(track, position, model::IClips(), track, position, originals);
}

AClipEdit::ClipsWithPosition AClipEdit::findClips(const model::TrackPtr& track, pts left, pts right)
{
    VAR_DEBUG(track)(left)(right);
    model::IClipPtr removePosition = model::IClipPtr();
    model::IClipPtr to = model::IClipPtr();       // Default: at end
    model::IClipPtr from = track->getClip(left);
    if (from)
    {
        ASSERT_EQUALS(from->getLeftPts(),left);

        // Remove until the clip BEFORE to
        to = track->getClip(right);

        ASSERT(!to || to->getLeftPts() == right)(to)(right); // Null ptr indicates at end
    }
    // else: Clips are added 'beyond' the current track length

    model::IClips::const_iterator it = track->getClips().begin();
    model::IClips removedClips;
    while (it != track->getClips().end() && *it != from)
    {
        ++it;
    }
    while (it != track->getClips().end() && *it != to)
    {
        removedClips.push_back(*it);
        ++it;
    }
    return make_pair(removedClips,to);
}

void AClipEdit::shiftAllTracks(pts start, pts amount, const model::Tracks& exclude)
{
    if (amount == 0) return;
    model::Tracks videoTracks = getTimeline().getSequence()->getVideoTracks();
    model::Tracks audioTracks = getTimeline().getSequence()->getAudioTracks();
    for ( model::TrackPtr track : exclude )
    {
        model::Tracks::iterator itVideo = find(videoTracks.begin(), videoTracks.end(), track);
        if (itVideo != videoTracks.end()) videoTracks.erase(itVideo);
        model::Tracks::iterator itAudio = find(audioTracks.begin(), audioTracks.end(), track);
        if (itAudio != audioTracks.end()) audioTracks.erase(itAudio);
    }
    shiftTracks(videoTracks, start, amount);
    shiftTracks(audioTracks, start, amount);
}

void AClipEdit::shiftTracks(const model::Tracks& tracks, pts start, pts amount)
{
    ASSERT_NONZERO(amount);
    for ( model::TrackPtr track : tracks )
    {
        if (amount > 0)
        {
            model::IClipPtr clip = track->getClip(start);
            if (clip)
            {
                model::IClipPtr clone = make_cloned<model::IClip>(clip);
                model::IClips newClips = { clone, boost::make_shared<model::EmptyClip>(amount) };
                replaceClip(clip, newClips);
            }
            // else: Beyond track length, no need to add a clip
        }
        else // (amount < 0)
        {
            model::IClipPtr clip = track->getClip(start);
            if (clip)
            {
                ASSERT(clip->isA<model::EmptyClip>());
                ASSERT_MORE_THAN_EQUALS(start,clip->getLeftPts());  // Enough room must be available for the shift
                ASSERT_LESS_THAN_EQUALS(start,clip->getRightPts()); // Enough room must be available for the shift
                model::IClips newClips = { boost::make_shared<model::EmptyClip>(clip->getLength() + amount) }; // NOTE: amount < 0
                replaceClip(clip, newClips);
            }
            // else: Beyond track length, no need to remove
        }
    }
}

model::IClipPtr AClipEdit::addTransition(const model::IClipPtr& leftClip, const model::IClipPtr& rightClip, const model::TransitionPtr& transition )
{
    model::TrackPtr track;
    model::IClipPtr position;

    ASSERT(  !leftClip ||  !leftClip->isA<model::Transition>() );
    ASSERT(  !leftClip ||  !leftClip->isA<model::EmptyClip>() );
    ASSERT( !rightClip || !rightClip->isA<model::Transition>() );
    ASSERT( !rightClip || !rightClip->isA<model::EmptyClip>() );
    ASSERT( !rightClip || !leftClip || ((leftClip->getNext() == rightClip) && (rightClip->getPrev() ==  leftClip)) );

    boost::optional<pts> left = transition->getLeft();
    boost::optional<pts> right = transition->getRight();

    if (left)
    {
        ASSERT(leftClip);

        // Determine position of transition
        track = leftClip->getTrack();
        position = leftClip->getNext();

        // Determine adjustment and adjust left clip
        pts adjustment = -1 * *left;
        ASSERT_MORE_THAN_EQUALS( adjustment, leftClip->getMinAdjustEnd() );
        ASSERT_LESS_THAN_EQUALS( adjustment, leftClip->getMaxAdjustEnd() );
        model::IClipPtr updatedLeft = make_cloned<model::IClip>(leftClip);
        updatedLeft->adjustEnd(adjustment);
        replaceClip(leftClip, { updatedLeft });
        VAR_DEBUG(updatedLeft);
    }
    if (right)
    {
        ASSERT(rightClip);

        // Determine position of transition
        track = rightClip->getTrack();

        // Determine adjustment and adjust right clip
        pts adjustment = *right;
        ASSERT_MORE_THAN_EQUALS( adjustment, rightClip->getMinAdjustBegin() );
        ASSERT_LESS_THAN_EQUALS( adjustment, rightClip->getMaxAdjustBegin() );
        model::IClipPtr updatedRight = make_cloned<model::IClip>(rightClip);
        updatedRight->adjustBegin(adjustment);
        replaceClip(rightClip, { updatedRight });

        // Determine position of transition
        position = updatedRight;
        VAR_DEBUG(updatedRight);
    }
    ASSERT(track);
    // NOT: ASSERT(position) - Transition may be added after last clip in track
    addClip(transition, track, position);
    return transition;
}

void AClipEdit::removeTransition(const model::TransitionPtr& transition)
{
    // Delete the transition and the underlying clips
    ASSERT_MORE_THAN_ZERO(transition->getLength());
    replaceClip(transition, { boost::make_shared<model::EmptyClip>(transition->getLength()) });
}

model::IClips AClipEdit::unapplyTransition(const model::TransitionPtr& transition, bool replacelinkedclipsalso )
{
    auto cloneLinkIfRequired = [this,replacelinkedclipsalso](model::IClipPtr link)
    {
        if (replacelinkedclipsalso && link && link->getTrack())
        {
            // If
            // - the scenario requires links to be replaced also (to avoid duplicate link relations when replacing only one of the linked clips), AND
            // - there is a link to the split clip, AND
            // - that clip is not replaced yet,
            // Ensure that that link is replaced with a clone.
            //
            // Note that the link may already have been removed from its track, if an operation was done that
            // applies to multiple tracks (typical example: split all tracks at certain pts position)
            replaceClip(link, { make_cloned(link) });
        }
    };
    model::IClips replacements;
    boost::optional<pts> left = transition->getLeft();
    if (left)
    {
        model::IClipPtr prev = transition->getPrev();
        ASSERT(prev);
        model::IClipPtr replacement;
        if (prev->isA<model::EmptyClip>())
        {
            // Extend empty clip
            replacement = boost::make_shared<model::EmptyClip>(prev->getLength() + *left);
        }
        else
        {
            // Extend the left clip
            replacement = make_cloned<model::IClip>(prev);
            replacement->adjustEnd(*left);
        }
        replaceClip(prev, { replacement });
        replacements.push_back(replacement);

        cloneLinkIfRequired(prev->getLink());
    }
    boost::optional<pts> right = transition->getRight();
    if (right)
    {
        model::IClipPtr next = transition->getNext();
        ASSERT(next);
        model::IClipPtr replacement;
        if (next->isA<model::EmptyClip>())
        {
            // Extend empty clip
            replacement = boost::make_shared<model::EmptyClip>(next->getLength() + *right);
        }
        else
        {
            // Extend next clip with right length of the transition
            replacement = make_cloned<model::IClip>(next);
            replacement->adjustBegin(-1 * *right);
        }
        replaceClip(next, { replacement });
        replacements.push_back(replacement);

        cloneLinkIfRequired(next->getLink());
    }
    removeClip(transition);
    return replacements;
}

model::IClipPtr AClipEdit::replaceWithEmpty(const model::IClips& clips)
{
    model::TrackPtr track = clips.front()->getTrack(); // Any clip will do, they're all part of the same track
    model::IClipPtr position = clips.back()->getNext(); // Position equals the clips after the last clip. May be 0.

    // Ensure that for regions the 'extra' space for transitions is added.
    // Basically the 'extra' space at the beginning of the first clip and the extra
    // space at the ending of the last clip must be added to the region.
    model::EmptyClipPtr empty = model::EmptyClip::replace(clips);

    //      ================== ADD ======================   ======= REMOVE =======
    newMove(track, position, { empty }, track, position, clips);

    return empty;
}

void AClipEdit::animatedDeleteAndTrim(const model::IClips& clipsToBeRemoved)
{
    model::MoveParameters undo;

    // Replace clips with empty area
    model::IClips emptyareas;
    for ( model::IClipPtr clip : clipsToBeRemoved )
    {
        model::TrackPtr track = clip->getTrack();
        ASSERT(track);

        model::IClipPtr empty = boost::make_shared<model::EmptyClip>(clip->getLength());
        emptyareas.push_back(empty);

        model::MoveParameterPtr move = boost::make_shared<model::MoveParameter>(track, clip->getNext(), model::IClips({ empty }), track, clip->getNext(), model::IClips({ clip }));
        undo.insert(undo.begin(),move->make_inverted()); // Must be executed in reverse order
        doMove(move);
    }

    // Now show the animation (gradually decrease the size of the empty areas)
    model::IClips mEmpties = emptyareas;
    model::IClips newempties;
    static const int SleepTimePerStep = 25;
    static const int AnimationDurationInMs = 150;
    static const int NumberOfSteps = AnimationDurationInMs / SleepTimePerStep;
    for (int step = NumberOfSteps - 1; step >= 0; --step)
    {
        for ( model::IClipPtr old : mEmpties )
        {
            model::TrackPtr track = old->getTrack();
            ASSERT(track);

            boost::rational<pts> oldFactor(step+1,NumberOfSteps);
            boost::rational<pts> newFactor(step,NumberOfSteps);
            boost::rational<pts> newlenrational( boost::rational<pts>(old->getLength(),1) / oldFactor * newFactor );
            pts newlen = static_cast<pts>(floor(boost::rational_cast<double>(newlenrational)));
            VAR_INFO(step)(old->getLength())(newlen)(newlenrational);
            model::IClipPtr empty = model::EmptyClipPtr(new model::EmptyClip(newlen));

            model::MoveParameterPtr move = boost::make_shared<model::MoveParameter>(track, old->getNext(), model::IClips({ empty }), track, old->getNext(), model::IClips({ old }));
            undo.insert(undo.begin(), move->make_inverted()); // Must be executed in reverse order
            doMove(move);
        }
        mEmpties = newempties;
        newempties.clear();
        getTimeline().Update();
        boost::this_thread::sleep(boost::posix_time::milliseconds(SleepTimePerStep));
    }

    getTimeline().beginTransaction();
    // Undo all the (temporary) changes for the animation
    for ( model::MoveParameterPtr move : undo )
    {
        doMove(move);
    }
    // Do the actual change
    for ( model::IClipPtr clip : clipsToBeRemoved )
    {
        removeClip(clip);
    }
}

model::IClips AClipEdit::splitTracksAndFindClipsToBeRemoved(const PtsIntervals& removed)
{
    model::IClips result;
    for ( PtsInterval interval : removed )
    {
        pts first = interval.lower();
        pts last = interval.upper();// - 1;
        ASSERT_LESS_THAN(first,last);
        for ( model::TrackPtr track : getSequence()->getTracks() )
        {
            split(track, first);
            split(track, last);
            model::IClips removedInTrack;
            for ( model::IClipPtr clip : track->getClips() )
            {
                if (clip->getLeftPts() >= last)
                {
                    break;
                }
                if (clip->getLeftPts() >= first)
                {
                    removedInTrack.push_back(clip);
                }
            }
            UtilVector<model::IClipPtr>(result).addElements(removedInTrack, model::IClipPtr());
        }
    }
    return result;
}

void AClipEdit::storeSelection()
{
    model::IClips selected;
    for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
    {
        for ( model::IClipPtr clip : track->getClips() )
        {
            if (clip->getSelected())
            {
                selected.push_back(clip);
            }
        }
    }
    mSelected.reset(selected);
}

void AClipEdit::restoreSelection()
{
    ASSERT(mSelected);
    getTimeline().getSelection().change(*mSelected);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AClipEdit::newMove(
    const model::TrackPtr& addTrack,
    const model::IClipPtr& addPosition,
    const model::IClips& addClips,
    const model::TrackPtr& removeTrack,
    const model::IClipPtr& removePosition,
    const model::IClips& removeClips)
{
    if (addClips.size() > 0)
    {
        if (removeClips.size() > 0)
        {
            LOG_DEBUG << DUMP(removeClips) << DUMP(removeTrack) << DUMP(removePosition) << DUMP(addClips) << DUMP(addTrack) << DUMP(addPosition);
        }
        else
        {
            LOG_DEBUG << DUMP(addClips) << DUMP(addTrack) << DUMP(addPosition);
        }
    }
    else
    {
        LOG_DEBUG << DUMP(removeClips) << DUMP(removeTrack) << DUMP(removePosition);
    }
    model::MoveParameterPtr move = boost::make_shared<model::MoveParameter>(addTrack, addPosition, addClips, removeTrack, removePosition, removeClips);
    mParams.push_back(move);
    mParamsUndo.insert(mParamsUndo.begin(), move->make_inverted()); // Must be executed in reverse order
    doMove(move);
}

void AClipEdit::doMove(const model::MoveParameterPtr& move)
{
    if (move->addTrack == move->removeTrack)
    {
        move->addTrack->replaceClips(move->removeClips, move->addClips, move->addPosition);
    }
    else
    {
        if (!move->removeClips.empty())
        {
            move->removeTrack->removeClips(move->removeClips);
        }
        if (!move->addClips.empty())
        {
            move->addTrack->addClips(move->addClips,move->addPosition);
        }
    }
}

void AClipEdit::avoidDanglingLinks()
{
    LOG_DEBUG;
    for (auto danglinglink : mReplacements->danglingLinks())
    {
        // If a link existed, but the original link wasn't replaced, then ensure that
        // a clone of that link is added to the list of possibly to be linked clips.
        model::IClipPtr clone = make_cloned<model::IClip>(danglinglink);
        clone->setLink(model::IClipPtr()); // The clone is linked to nothing, since linking is done below.
        ASSERT(danglinglink->getTrack())(danglinglink); // Must still be part of a track (thus, can't have been removed, because then it would have to be part of the map)
        replaceClip(danglinglink, { clone }); // std::map iterators - foreach - remain valid
    }
}

void AClipEdit::mergeConsecutiveEmptyClips()
{
    LOG_DEBUG;
    // PERF make 'dirty' mechanism to avoid useless updates: each track is clean, whenever a move is done, it becomes dirty. When AClipEdit has finalized its transition all tracks are 'clean' again
    mergeConsecutiveEmptyClips(getTimeline().getSequence()->getVideoTracks());
    mergeConsecutiveEmptyClips(getTimeline().getSequence()->getAudioTracks());
}

void AClipEdit::mergeConsecutiveEmptyClips(const model::Tracks& tracks)
{
    auto replace = [this](model::TrackPtr track, model::IClips& clips, pts length)
    {
        if (clips.size() == 1 && length > 0) { return; } // Do not replace 1 empty clip (no merging required), except if this one clip has a length of 0 (then this clip may be removed)
        model::IClipPtr position = clips.back()->getNext(); // Position equals the clips after the last clip. May be 0.
        model::IClips replacement;
        if (length > 0)
        {
            // Ensure that for regions the 'extra' space for transitions is added.
            // Basically the 'extra' space at the beginning of the first clip and the extra
            // space at the ending of the last clip must be added to the region.
            replacement.push_back(model::EmptyClip::replace(clips));
        }
        // else: Simply replace with an empty list, thus remove the clip(s)

        //      ================== ADD =====   ======= REMOVE =======
        newMove(track, position, replacement, track, position, clips);
    };

    for ( model::TrackPtr track : tracks )
    {
        pts length = 0;
        bool inregion = false;
        model::IClips removed;

        model::IClips clips = track->getClips(); // Take copy: list of clips in track is modified in the next loop
        for ( model::IClipPtr clip : clips )
        {
            if (clip->isA<model::EmptyClip>())
            {
                inregion = true;
                length += clip->getLength();
                removed.push_back(clip);
            }
            else // !clip->isA<model::EmptyClip>()
            {
                if (inregion)
                {
                    replace(track,removed,length);
                    length = 0;
                    removed.clear();
                }
                inregion = false;
            }
        }
        if (inregion)
        {
            replace(track,removed,length);
        }
    }
}

void AClipEdit::removeEmptyClipsAtEndOfTracks()
{
    for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
    {
        model::IClips clips = track->getClips(); // Take copy: list of clips in track is modified in the next loop
        if (!clips.empty())
        {
            model::IClips::iterator it = clips.end();
            --it;
            if ((*it)->isA<model::EmptyClip>())
            {
                removeClip(*it);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AClipEdit& obj)
{
    os << static_cast<const ATimelineCommand&>(obj);
    return os;
}

}}} // namespace
