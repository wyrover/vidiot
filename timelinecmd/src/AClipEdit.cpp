#include "AClipEdit.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Clip.h"
#include "Cursor.h"
#include "CrossFade.h"
#include "EmptyClip.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackEvent.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

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
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool AClipEdit::Do()
{
    VAR_INFO(*this)(mInitialized);

    if (!mInitialized)
    {
        initialize();

        mergeConsecutiveEmptyClips();

        mInitialized = true;
    }
    else
    {
        ASSERT_NONZERO(mParams.size());
        BOOST_FOREACH( model::MoveParameterPtr move, mParams )
        {
            doMove(move);
        }
    }

    // This is required to
    // - reset model::Track iterators
    // - start at the last played position (and not start at the "buffered" position)
    getTimeline().getCursor().moveCursorOnUser(getTimeline().getCursor().getPosition());

    return true;
}

bool AClipEdit::Undo()
{
    VAR_INFO(*this);
    ASSERT_NONZERO(mParamsUndo.size());
    BOOST_FOREACH( model::MoveParameterPtr move, mParamsUndo )
    {
        doMove(move);
    }

    // This is required to
    // - reset model::Track iterators
    // - start at the last played position (and not start at the "buffered" position)
    getTimeline().getCursor().moveCursorOnUser(getTimeline().getCursor().getPosition());

    return true;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS FOR SUBCLASSES
//////////////////////////////////////////////////////////////////////////

void AClipEdit::split(model::TrackPtr track, pts position, ReplacementMap* conversionmap)
{
    model::IClipPtr clip = track->getClip(position);
    if (clip)
    {
        if (clip->isA<model::Transition>())
        {
            // There is a transition at the given position. Remove it.
            removeClip(clip);
        }
        model::IClipPtr clip = track->getClip(position);
        if (clip) // If there is a clip at the given position, it might need to be split
        {
            ASSERT(clip->isA<model::IClip>())(clip);
            position -= clip->getLeftPts();
            if (position != 0) // If there is already a cut at the given position, nothing is changed.
            {
                ASSERT_LESS_THAN(position,clip->getLength());
                model::IClipPtr left = make_cloned<model::IClip>(clip);
                model::IClipPtr right = make_cloned<model::IClip>(clip);
                left->adjustEnd(position - clip->getLength());
                right->adjustBegin(position);
                model::IClips replacements = boost::assign::list_of(left)(right);
                replaceClip(clip, replacements, conversionmap);
            }

        }
    }
}

void AClipEdit::replaceClip(model::IClipPtr original, model::IClips replacements, ReplacementMap* conversionmap)
{
    model::TrackPtr track = original->getTrack();
    ASSERT(track);
    model::IClipPtr position = original->getNext();
    model::IClips originallist = boost::assign::list_of(original);

    if (conversionmap)
    {
        ASSERT_MAP_CONTAINS_NOT((*conversionmap),original);
        (*conversionmap)[ original ] = replacements;
    }

    //      ============= ADD ===========  ========== REMOVE ===========
    newMove(track, position, replacements, track, position, originallist);
}

void AClipEdit::removeClip(model::IClipPtr original)
{
    replaceClip(original, model::IClips());
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

void AClipEdit::replaceLinks(ReplacementMap& conversionmap)
{
    // For all replaced clips, ensure that the linked clip is also replaced,
    // at least with just a plain clone of the original link. This is needed to
    // avoid having these links 'dangling' after removal.
    BOOST_FOREACH( ReplacementMap::value_type link, conversionmap )
    {
        model::IClipPtr original = link.first;
        model::IClipPtr originallink = original->getLink();
        if (originallink && conversionmap.find(originallink) == conversionmap.end())
        {
            // If a link existed, but the original link wasn't already cut, then ensure that
            // a clone of that link is added to the list of possibly to be linked clips.
            model::IClipPtr clone = make_cloned<model::IClip>(originallink);
            clone->setLink(model::IClipPtr()); // The clone is linked to nothing, since linking is done below.
            replaceClip(originallink, boost::assign::list_of(clone), &conversionmap);
        }
    }

    // At this point all clips AND their original links (or their clones)
    // are part of the replacement map. Now the replacements for a clip
    // are linked to the replacements of its link.
    BOOST_FOREACH( ReplacementMap::value_type link, conversionmap )
    {
        model::IClipPtr clip1 = link.first;
        model::IClips new1 = link.second;
        model::IClips::iterator it1 = new1.begin();

        model::IClipPtr clip2 = clip1->getLink();
        model::IClips new2 = conversionmap[clip2];
        model::IClips::iterator it2 = new2.begin();

        while ( it1 != new1.end() && it2 != new2.end() )
        {
            (*it1)->setLink(*it2);
            (*it2)->setLink(*it1);
            ++it1;
            ++it2;
        }
        // For all remaining clips in both lists: not linked.
        while ( it1 != new1.end() )
        {
            (*it1)->setLink(model::IClipPtr());
            ++it1;
        }
        while ( it2 != new2.end() )
        {
            (*it2)->setLink(model::IClipPtr());
            ++it2;
        }
    }
}

void AClipEdit::mergeConsecutiveEmptyClips()
{
    mergeConsecutiveEmptyClips(getTimeline().getSequence()->getVideoTracks());
    mergeConsecutiveEmptyClips(getTimeline().getSequence()->getAudioTracks());
}

void AClipEdit::mergeConsecutiveEmptyClips(model::Tracks tracks)
{
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
                    //      ======================================= ADD =====================================  ====== REMOVE ======
                    newMove(track, clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(length)), track, clip, removed);
                    length = 0;
                    removed.clear();
                }
                inregion = false;
            }
        }
        if (inregion)
        {
            //      ======================================= ADD =====================================
            newMove(track, model::IClipPtr(), boost::assign::list_of(boost::make_shared<model::EmptyClip>(length)), track, model::IClipPtr(), removed);
        }
    }
}

void AClipEdit::newMove(model::TrackPtr addTrack, model::IClipPtr addPosition, model::IClips addClips, model::TrackPtr removeTrack, model::IClipPtr removePosition, model::IClips removeClips)
{
    VAR_DEBUG(addTrack)(addPosition)(addClips)(removeTrack)(removePosition)(removeClips);
    model::MoveParameterPtr move = boost::make_shared<model::MoveParameter>(addTrack, addPosition, addClips, removeTrack, removePosition, removeClips);
    mParams.push_back(move);
    mParamsUndo.push_front(move->make_inverted()); // push_front: Must be executed in reverse order
    doMove(move);
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
            model::IClipPtr clone = make_cloned<model::IClip>(clip);
            model::IClips newClips = makeEmptyClips(amount);
            newClips.push_front(clone);
            replaceClip(clip, newClips);
        }
        else // (amount < 0)
        {
            model::IClipPtr clip = track->getClip(start);
            ASSERT(clip->isA<model::EmptyClip>());
            ASSERT_MORE_THAN_EQUALS(start,clip->getLeftPts());  // Enough room must be available for the shift
            ASSERT_LESS_THAN_EQUALS(start,clip->getRightPts()); // Enough room must be available for the shift
            replaceClip(clip, makeEmptyClips(clip->getLength() + amount));  // NOTE: amount < 0
        }
    }
}

model::IClipPtr AClipEdit::makeEmptyClip(pts length)
{
    return boost::static_pointer_cast<model::IClip>(boost::make_shared<model::EmptyClip>(length));
}

model::IClips AClipEdit::makeEmptyClips(pts length)
{
    return boost::assign::list_of(makeEmptyClip(length));
}

model::IClipPtr AClipEdit::makeTransition( model::IClipPtr leftClip, pts leftLength, model::IClipPtr rightClip, pts rightLength, ReplacementMap& conversionmap )
{
    model::TrackPtr track;
    model::IClipPtr position;

    ASSERT( !leftClip || ((rightClip->getPrev() ==  leftClip) && ! leftClip->isA<model::Transition>()) );
    ASSERT(!rightClip || (( leftClip->getNext() == rightClip) && !rightClip->isA<model::Transition>()) );

    // todo if left or right clips are transitions, then do not add transitions over them again. Then only add transition to the other end.

    if (leftLength > 0)
    {
        // Determine position of transition
        track = leftClip->getTrack();
        position = leftClip->getNext();

        // Determine adjustment and adjust left clip
        model::IClipPtr updatedLeft = make_cloned<model::IClip>(leftClip);
        pts adjustment = -leftLength;
        adjustment = std::max( adjustment, updatedLeft->getMinAdjustEnd() ); // todo this should be handled by the command, not here. + add asserts in this method checking these bounds
        adjustment = std::min( adjustment, updatedLeft->getMaxAdjustEnd() ); // todo this should be handled by the command, not here. + add asserts in this method checking these bounds
        updatedLeft->adjustEnd(adjustment);
        replaceClip(leftClip,boost::assign::list_of(updatedLeft),&conversionmap);
        VAR_DEBUG(updatedLeft);
    }
    if (rightLength > 0)
    {
        // Determine position of transition
        track = rightClip->getTrack();

        // Determine adjustment and adjust right clip
        model::IClipPtr updatedRight = make_cloned<model::IClip>(rightClip);
        pts adjustment = rightLength;
        adjustment = std::max( adjustment, updatedRight->getMinAdjustBegin() ); // todo this should be handled by the command, not here. + add asserts in this method checking these bounds
        adjustment = std::min( adjustment, updatedRight->getMaxAdjustBegin() ); // todo this should be handled by the command, not here. + add asserts in this method checking these bounds
        updatedRight->adjustBegin(adjustment);
        replaceClip(rightClip,boost::assign::list_of(updatedRight),&conversionmap);
        
        // Determine position of transition
        position = updatedRight;
        VAR_DEBUG(updatedRight);
    }
    ASSERT(track);
    ASSERT(position);
    model::IClipPtr transition = boost::make_shared<model::transition::CrossFade>(leftLength, rightLength); 
    newMove(track,position,boost::assign::list_of(transition));
    return transition;
}

void AClipEdit::removeTransition( model::TransitionPtr transition, ReplacementMap& conversionmap )
{
    // Delete the transition and the underlying clips
    ASSERT_MORE_THAN_ZERO(transition->getLength());
    replaceClip(transition, makeEmptyClips(transition->getLength()), &conversionmap);
}

void AClipEdit::unapplyTransition( model::TransitionPtr transition, ReplacementMap& conversionmap )
{
    model::IClips replacements;
    if (transition->getLeft() > 0)
    {
        model::IClipPtr prev = transition->getPrev(); // todo make prev and next methods part of iclip
        ASSERT(prev && !prev->isA<model::EmptyClip>());
        // Extend previous clip with left length of the transition
        model::IClipPtr adjustedLeft = make_cloned<model::IClip>(prev);
        adjustedLeft->adjustEnd(transition->getLeft());
        replaceClip(prev,boost::assign::list_of(adjustedLeft),&conversionmap);
    }
    if (transition->getRight() > 0)
    {
        model::IClipPtr next = transition->getNext();
        ASSERT(next && !next->isA<model::EmptyClip>());
        // Extend next clip with right length of the transition
        model::IClipPtr adjustedRight = make_cloned<model::IClip>(next);
        adjustedRight->adjustBegin(-transition->getRight());
        replaceClip(next,boost::assign::list_of(adjustedRight),&conversionmap);
    }
    removeClip(transition);
}

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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AClipEdit& obj )
{
    os << static_cast<const ATimelineCommand&>(obj);
    return os;
}

}}} // namespace
