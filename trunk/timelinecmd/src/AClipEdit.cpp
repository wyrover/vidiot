#include "AClipEdit.h"

#include "Clip.h"
#include "Config.h"
#include "VideoTransition_CrossFade.h"
#include "Cursor.h"
#include "Dump.h"
#include "EmptyClip.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackEvent.h"
#include "Transition.h"
#include "UtilList.h"

#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSet.h"

namespace gui { namespace timeline { namespace command {

    //////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AClipEdit::AClipEdit(model::SequencePtr sequence)
    :   ATimelineCommand(sequence)
    ,   mParams()
    ,   mParamsUndo()
    ,   mInitialized(false)
{
    VAR_INFO(this);
}

AClipEdit::~AClipEdit()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void AClipEdit::doExtra()
{
}

void AClipEdit::undoExtra()
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
    mReplacements.clear();
    mExpandedReplacements.clear();
    mInitialized = false;
}

void AClipEdit::newMove(model::TrackPtr addTrack, model::IClipPtr addPosition, model::IClips addClips, model::TrackPtr removeTrack, model::IClipPtr removePosition, model::IClips removeClips)
{
    VAR_DEBUG(addTrack)(addPosition)(addClips)(removeTrack)(removePosition)(removeClips);
    model::MoveParameterPtr move = boost::make_shared<model::MoveParameter>(addTrack, addPosition, addClips, removeTrack, removePosition, removeClips);
    mParams.push_back(move);
    mParamsUndo.push_front(move->make_inverted()); // push_front: Must be executed in reverse order
    doMove(move);
}

void AClipEdit::split(model::TrackPtr track, pts position)
{
    model::IClipPtr clip = track->getClip(position);
    if (clip && clip->getLeftPts() < position)
    {
        // If there already is a cut at the given position (leftpts != position) then there's no need to cut again.
        // Furthermore, doing the check at the beginning makes the handling of transitions (and the possible
        // 'unapplying of them' a bit simpler in the code below (no more checks for 'if already cut' required.

        model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
        if (transition)
        {
            // There is a transition at the given position: Restore the underlying clips.
            ASSERT_LESS_THAN(transition->getLeftPts(),position);
            unapplyTransition(transition,true);
            clip = track->getClip(position); // Must be done again, since unapplyTransition changes the track.
        }
        if (clip) // If there is a clip at the given position, it might need to be split
        {
            ASSERT(clip->isA<model::IClip>())(clip);
            ASSERT(!clip->isA<model::Transition>())(clip);
            position -= clip->getLeftPts();
            if (position != 0)
            {
                // If there is already a cut at the given position, nothing is changed.
                // Note that unapplying a transition creates a new cut, therefore this
                // check must be done again AFTER unapplyTransition has been applied.
                ASSERT_LESS_THAN(position,clip->getLength());
                model::IClipPtr left = make_cloned<model::IClip>(clip);
                model::IClipPtr right = make_cloned<model::IClip>(clip);
                left->adjustEnd(position - clip->getLength());
                right->adjustBegin(position);
                model::IClips replacements = boost::assign::list_of(left)(right);
                replaceClip(clip, replacements);
            }
        }
    }
}

void AClipEdit::replaceClip(model::IClipPtr original, model::IClips replacements, bool maintainlinks)
{
    model::TrackPtr track = original->getTrack();
    ASSERT(track);
    model::IClipPtr position = original->getNext();
    model::IClips originallist = boost::assign::list_of(original);

    if (maintainlinks)
    {
        ASSERT_MAP_CONTAINS_NOT((mReplacements),original);
        mReplacements[ original ] = replacements;
    }

    //      ============= ADD ===========  ========== REMOVE ===========
    newMove(track, position, replacements, track, position, originallist);
}

void AClipEdit::addClip(model::IClipPtr clip, model::TrackPtr track, model::IClipPtr position)
{
    ASSERT(!clip->getLink())(clip);
    addClips(boost::assign::list_of(clip),track,position);
}

void AClipEdit::addClips(model::IClips clips, model::TrackPtr track, model::IClipPtr position)
{
    newMove(track, position, clips );
}

void AClipEdit::removeClip(model::IClipPtr original)
{
    replaceClip(original, model::IClips());
}

void AClipEdit::removeClips(model::IClips originals)
{
    model::TrackPtr track = originals.front()->getTrack();
    ASSERT(track);
    model::IClipPtr position = originals.back()->getNext();

    BOOST_FOREACH( model::IClipPtr original, originals )
    {
        ASSERT_MAP_CONTAINS_NOT((mReplacements),original);
        mReplacements[ original ] = model::IClips();
    }

    //      ============== ADD =============  ======== REMOVE =========
    newMove(track, position, model::IClips(), track, position, originals);
}

AClipEdit::ClipsWithPosition AClipEdit::findClips(model::TrackPtr track, pts left, pts right)
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

void AClipEdit::shiftAllTracks(pts start, pts amount, model::Tracks exclude)
{
    if (amount == 0) return;
    model::Tracks videoTracks = getTimeline().getSequence()->getVideoTracks();
    model::Tracks audioTracks = getTimeline().getSequence()->getAudioTracks();
    BOOST_FOREACH( model::TrackPtr track, exclude )
    {
        model::Tracks::iterator itVideo = find(videoTracks.begin(), videoTracks.end(), track);
        if (itVideo != videoTracks.end()) videoTracks.erase(itVideo);
        model::Tracks::iterator itAudio = find(audioTracks.begin(), audioTracks.end(), track);
        if (itAudio != audioTracks.end()) audioTracks.erase(itAudio);
    }
    shiftTracks(videoTracks, start, amount);
    shiftTracks(audioTracks, start, amount);
}

void AClipEdit::shiftTracks(model::Tracks tracks, pts start, pts amount)
{
    ASSERT_NONZERO(amount);
    BOOST_FOREACH( model::TrackPtr track, tracks )
    {
        if (amount > 0)
        {
            model::IClipPtr clip = track->getClip(start);
            if (clip)
            {
                model::IClipPtr clone = make_cloned<model::IClip>(clip);
                model::IClips newClips = boost::assign::list_of(boost::make_shared<model::EmptyClip>(amount));
                newClips.push_front(clone);
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
                model::IClips newClips = boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getLength() + amount)); // NOTE: amount < 0
                replaceClip(clip, newClips);
            }
            // else: Beyond track length, no need to remove
        }
    }
}

model::IClipPtr AClipEdit::makeTransition( model::IClipPtr leftClip, pts leftLength, model::IClipPtr rightClip, pts rightLength )
{
    model::TrackPtr track;
    model::IClipPtr position;

    ASSERT(  !leftClip ||  !leftClip->isA<model::Transition>() );
    ASSERT(  !leftClip ||  !leftClip->isA<model::EmptyClip>() );
    ASSERT( !rightClip || !rightClip->isA<model::Transition>() );
    ASSERT( !rightClip || !rightClip->isA<model::EmptyClip>() );
    ASSERT( !rightClip || !leftClip || ((leftClip->getNext() == rightClip) && (rightClip->getPrev() ==  leftClip)) );

    if (leftLength > 0)
    {
        ASSERT(leftClip);

        // Determine position of transition
        track = leftClip->getTrack();
        position = leftClip->getNext();

        // Determine adjustment and adjust left clip
        pts adjustment = -leftLength;
        ASSERT_MORE_THAN_EQUALS( adjustment, leftClip->getMinAdjustEnd() );
        ASSERT_LESS_THAN_EQUALS( adjustment, leftClip->getMaxAdjustEnd() );
        model::IClipPtr updatedLeft = make_cloned<model::IClip>(leftClip);
        updatedLeft->adjustEnd(adjustment);
        replaceClip(leftClip,boost::assign::list_of(updatedLeft));
        VAR_DEBUG(updatedLeft);
    }
    if (rightLength > 0)
    {
        ASSERT(rightClip);

        // Determine position of transition
        track = rightClip->getTrack();

        // Determine adjustment and adjust right clip
        pts adjustment = rightLength;
        ASSERT_MORE_THAN_EQUALS( adjustment, rightClip->getMinAdjustBegin() );
        ASSERT_LESS_THAN_EQUALS( adjustment, rightClip->getMaxAdjustBegin() );
        model::IClipPtr updatedRight = make_cloned<model::IClip>(rightClip);
        updatedRight->adjustBegin(adjustment);
        replaceClip(rightClip,boost::assign::list_of(updatedRight));

        // Determine position of transition
        position = updatedRight;
        VAR_DEBUG(updatedRight);
    }
    ASSERT(track);
    ASSERT(position);
    model::IClipPtr transition = boost::make_shared<model::video::transition::CrossFade>(leftLength, rightLength);
    addClip(transition, track, position);
    return transition;
}

void AClipEdit::removeTransition( model::TransitionPtr transition )
{
    // Delete the transition and the underlying clips
    ASSERT_MORE_THAN_ZERO(transition->getLength());
    replaceClip(transition, boost::assign::list_of(boost::make_shared<model::EmptyClip>(transition->getLength())));
}

model::IClips AClipEdit::unapplyTransition( model::TransitionPtr transition, bool replacelinkedclipsalso )
{
    model::IClips replacements;
    if (transition->getLeft() > 0)
    {
        model::IClipPtr prev = transition->getPrev();
        ASSERT(prev);
        model::IClipPtr replacement;
        if (prev->isA<model::EmptyClip>())
        {
            // Extend empty clip
            replacement = boost::make_shared<model::EmptyClip>(prev->getLength() + transition->getLeft());
        }
        else
        {
            // Extend the left clip
            replacement = make_cloned<model::IClip>(prev);
            replacement->adjustEnd(transition->getLeft());
        }
        replaceClip(prev,boost::assign::list_of(replacement));
        replacements.push_back(replacement);

        if (replacelinkedclipsalso && prev->getLink())
        {
            replaceClip(prev->getLink(), boost::assign::list_of(make_cloned(prev->getLink())));
        }
    }
    if (transition->getRight() > 0)
    {
        model::IClipPtr next = transition->getNext();
        ASSERT(next);
        model::IClipPtr replacement;
        if (next->isA<model::EmptyClip>())
        {
            // Extend empty clip
            replacement = boost::make_shared<model::EmptyClip>(next->getLength() + transition->getRight());
        }
        else
        {
            // Extend next clip with right length of the transition
            replacement = make_cloned<model::IClip>(next);
            replacement->adjustBegin(-transition->getRight());
        }
        replaceClip(next,boost::assign::list_of(replacement));
        replacements.push_back(replacement);

        if (replacelinkedclipsalso && next->getLink())
        {
            replaceClip(next->getLink(), boost::assign::list_of(make_cloned(next->getLink())));
        }
    }
    removeClip(transition);
    return replacements;
}

model::IClipPtr AClipEdit::replaceWithEmpty(model::IClips clips)
{
    model::TrackPtr track = clips.front()->getTrack(); // Any clip will do, they're all part of the same track
    model::IClipPtr position = clips.back()->getNext(); // Position equals the clips after the last clip. May be 0.

    // Ensure that for regions the 'extra' space for transitions is added.
    // Basically the 'extra' space at the beginning of the first clip and the extra
    // space at the ending of the last clip must be added to the region.
    model::EmptyClipPtr empty = model::EmptyClip::replace(clips);

    //      ================== ADD ======================   ======= REMOVE =======
    newMove(track, position, boost::assign::list_of(empty), track, position, clips);

    return empty;
}

void AClipEdit::animatedTrimEmpty(model::IClips emptyareas)
{
    model::MoveParameters cachedParams = mParams;
    model::MoveParameters cachedParamsUndo = mParamsUndo;

    model::IClips mEmpties = emptyareas;
    model::IClips newempties;
    static const int SleepTimePerStep = 25;
    static const int AnimationDurationInMs = 250;
    static const int NumberOfSteps = AnimationDurationInMs / SleepTimePerStep;
    for (int step = NumberOfSteps - 1; step >= 0; --step)
    {
        BOOST_FOREACH( model::IClipPtr old, mEmpties )
        {
            boost::rational<pts> oldFactor(step+1,NumberOfSteps);
            boost::rational<pts> newFactor(step,NumberOfSteps);
            boost::rational<pts> newlenrational( boost::rational<pts>(old->getLength(),1) / oldFactor * newFactor );
            pts newlen = static_cast<pts>(floor(boost::rational_cast<double>(newlenrational)));
            VAR_INFO(step)(old->getLength())(newlen)(newlenrational);
            model::IClipPtr empty = model::EmptyClipPtr(new model::EmptyClip(newlen));
            newempties.push_back(empty);
            replaceClip(old,boost::assign::list_of(empty));
        }
        mEmpties = newempties;
        newempties.clear();
        boost::this_thread::sleep(boost::posix_time::milliseconds(SleepTimePerStep));
        wxSafeYield(); // Show update progress, but do not allow user input
    }
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool AClipEdit::Do()
{
    VAR_INFO(*this)(mInitialized);

    doExtra();

    if (!mInitialized)
    {
        initialize();

        mergeConsecutiveEmptyClips();

        avoidDanglingLinks();

        expandReplacements();

        replaceLinks();

        mInitialized = true;

        // The following are no longer required (avoid extra memory use):
        mReplacements.clear();
        mExpandedReplacements.clear();
    }
    else
    {
        ASSERT_NONZERO(mParams.size());
        BOOST_FOREACH( model::MoveParameterPtr move, mParams )
        {
            doMove(move);
        }
    }

    getTimeline().modelChanged();

    return true;
}

bool AClipEdit::Undo()
{
    VAR_INFO(*this)(mParamsUndo.size());
    //NOT: ASSERT_NONZERO(mParamsUndo.size()); - Due to the use in 'Revert()'

    BOOST_FOREACH( model::MoveParameterPtr move, mParamsUndo )
    {
        doMove(move);
    }

    undoExtra();

    getTimeline().modelChanged();

    return true;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AClipEdit::doMove(model::MoveParameterPtr move)
{
    if (move->removeClips.size() > 0)
    {
        move->removeTrack->removeClips(move->removeClips);
    }
    if (move->addClips.size() > 0)
    {
        move->addTrack->addClips(move->addClips,move->addPosition);
    }
}

void AClipEdit::avoidDanglingLinks()
{
    BOOST_FOREACH( auto link, mReplacements )
    {
        model::IClipPtr original = link.first;
        model::IClipPtr originallink = original->getLink();
        if (originallink && mReplacements.find(originallink) == mReplacements.end())
        {
            // If a link existed, but the original link wasn't replaced, then ensure that
            // a clone of that link is added to the list of possibly to be linked clips.
            model::IClipPtr clone = make_cloned<model::IClip>(originallink);
            clone->setLink(model::IClipPtr()); // The clone is linked to nothing, since linking is done below.
            ASSERT(originallink->getTrack())(originallink); // Must still be part of a track (thus, can't have been removed, because then it would have to be part of the map)
            replaceClip(originallink, boost::assign::list_of(clone)); // std::map iterators - foreach - remain valid
        }
    }
}

void AClipEdit::expandReplacements()
{
    std::set<model::IClipPtr> allReplacements;
    BOOST_FOREACH( auto entry, mReplacements )
    {
        allReplacements.insert(entry.second.begin(),entry.second.end());
    }
    BOOST_FOREACH( auto entry, mReplacements )
    {
        if (allReplacements.find(entry.first) == allReplacements.end())
        {
            // The expansion is done for all non-intermediate clips in the map. Since the replacement
            // map is only required for linking of clips, only the original clips and their replacement
            // are relevant. Intermediate clips are not.
            mExpandedReplacements[entry.first] = expandReplacements(entry.second);
        }
    }
}

model::IClips AClipEdit::expandReplacements(model::IClips original)
{
    model::IClips result;
    BOOST_FOREACH( model::IClipPtr clip, original )
    {
        if (mReplacements.find(clip) == mReplacements.end())
        {
            // The replacement clip is not replaced
            result.push_back(clip);
        }
        else
        {
            // The replacement clip has been replaced also
            model::IClips replacements = mReplacements[clip];
            result.splice(result.end(), expandReplacements(replacements));
        }
    }
    return result;
}

void AClipEdit::replaceLinks()
{
    LOG_DEBUG;
    BOOST_FOREACH( ReplacementMap::value_type link, mExpandedReplacements )
    {
        model::IClipPtr clip1 = link.first;
        model::IClips new1 = link.second;
        model::IClips::iterator it1 = new1.begin();

        model::IClipPtr clip2 = clip1->getLink();
        if (clip2) // The clip doesn't necessarily have a link with another clip
        {
            ASSERT(!clip2->isA<model::EmptyClip>())(clip2); // Linking to an empty clip is not allowed

            // If clip clip1 is replaced with replacement1, then its link link1 MUST also be replaced
            // (since link1 must be replaced with a clip whose link is replacement1). This must be guaranteed
            // by all AClipEdit derived classes (not solved generally in the base class since that would
            // cause lots of redundant replacements).
            ASSERT(mExpandedReplacements.find(clip2) != mExpandedReplacements.end())(clip1)(clip2)(mReplacements)(mExpandedReplacements);

            model::IClips new2 = mExpandedReplacements[clip2];
            model::IClips::iterator it2 = new2.begin();

            auto NoLinkingAllowed = [](model::IClipPtr clip)->bool
            {
                return (clip->isA<model::EmptyClip>() || clip->isA<model::Transition>()); // Linking to/from empty clips and transitions is not allowed. Skip these.
            };

            while ( it1 != new1.end() && it2 != new2.end() )
            {
                model::IClipPtr newclip1 = *it1;
                model::IClipPtr newclip2 = *it2;
                if (NoLinkingAllowed(newclip1))
                {
                    ++it1;
                    continue;
                }
                if (NoLinkingAllowed(newclip2))
                {
                    ++it2;
                    continue;
                }
                newclip1->setLink(newclip2);
                newclip2->setLink(newclip1);
                ++it1;
                ++it2;
            }
            // For all remaining clips in both lists: not linked.
            for (; it1 != new1.end(); ++it1)
            {
                (*it1)->setLink(model::IClipPtr());
            }
            for (; it2 != new2.end(); ++it2)
            {
                (*it2)->setLink(model::IClipPtr());
            }
        }
    }

    if (Config::ReadBool(Config::sPathTest))
    {
        // Only in test mode: verify all links.
        BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
        {
            BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
            {
                if (clip->getLink())
                {
                    if (clip != clip->getLink()->getLink())
                    {
                        getTimeline().getDump().dump();
                    }
                    ASSERT_EQUALS(clip, clip->getLink()->getLink());
                }
            }
        }
    }
}

void AClipEdit::mergeConsecutiveEmptyClips()
{
    LOG_DEBUG;
    // PERF make 'dirty' mechanism to avoid useless updates: each track is clean, whenever a move is done, it becomes dirty. When AClipEdit has finalized its transition all tracks are 'clean' again
    mergeConsecutiveEmptyClips(getTimeline().getSequence()->getVideoTracks());
    mergeConsecutiveEmptyClips(getTimeline().getSequence()->getAudioTracks());
}

void AClipEdit::mergeConsecutiveEmptyClips(model::Tracks tracks)
{
    auto replace = [this](model::TrackPtr track, model::IClips& clips, pts length)
    {
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

    BOOST_FOREACH( model::TrackPtr track, tracks )
    {
        pts length = 0;
        bool inregion = false;
        model::IClips removed;

        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AClipEdit& obj )
{
    os << static_cast<const ATimelineCommand&>(obj);
    return os;
}

}}} // namespace