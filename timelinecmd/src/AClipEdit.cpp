#include "AClipEdit.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "Track.h"
#include "Clip.h"
#include "EmptyClip.h"

namespace command {

AClipEdit::AClipEdit(gui::timeline::Timeline& timeline)
:   TimelineCommand(timeline)
,   mParams()
,   mParamsUndo()
{
    VAR_INFO(this);
}

AClipEdit::~AClipEdit()
{
}

bool AClipEdit::Do()
{
    VAR_INFO(this);
todo redo
    bool redo = (mParamsUndo.size() != 0);

    if (mParams.size() == 0)
    {
        // This can only happen in case of the 'new' drag/drop constructor
        //
        // 1. Make move objects for replacing all 'dragged' clips with EmptyClips.
        //    The original dragged clips remain linked to their original links
        //    (since these are dragged also, by design).
        // 2. Execute these moves (since the following moves depend on these newly
        //    created EmptyClips).
        // 3. Make move objects for pasting all Drops. 
        //    Ensure that the 'replacement map' contains a mapping of original->replacement
        //    for clips in the timeline that are 'split' in two because of the drop. Note that
        //    this is not requrired for the dropped clips themselves -> they remain linked
        //    to the same linked clips.
        // 4. Execute those drops.
        // 5. Make moves for merging all consecutive EmptyClips.
        // 6. Ensure that links are maintained. 

        ReplacementMap mConversion;

        LOG_DEBUG << "STEP 1 & 2: Replace all drags with EmptyClips";
        BOOST_FOREACH( model::ClipPtr clip, mDrags )
        {
            replaceClip(clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getNumberOfFrames())));
        }

        LOG_DEBUG << "STEP 3 & 4: Execute the drops AND fill replacement map";
        BOOST_FOREACH( PasteInfo drop, mDrops )
        {
            ASSERT(drop.position >= 0)(drop.position);
            ASSERT(drop.track);
            ASSERT(drop.clips.size() != 0);
            VAR_DEBUG(drop.position)(drop.track)(drop.clips);

            // Determine end pts of dropped clips
            pts dropEndPosition = drop.position;
            BOOST_FOREACH( model::ClipPtr clip, drop.clips )
            {
                dropEndPosition += clip->getNumberOfFrames();
            }

            // Ensure that the track has cuts at the begin and the end of the dropped clips
            split(drop.track, drop.position,   &mConversion);
            split(drop.track, dropEndPosition, &mConversion);

            // Determine the clips to be replaced.
            // Done AFTER the splitting above, since that requires clip addition/removal.
            model::ClipPtr removePosition = model::ClipPtr();
            model::Clips removedClips;
            model::ClipPtr to = model::ClipPtr();
            model::ClipPtr from = drop.track->getClip(drop.position);
            if (from)
            {
                // Clips are added 'inside' the track
                ASSERT(from->getLeftPts() == drop.position)(from)(drop.position);

                model::ClipPtr firstNotReplacedClip = drop.track->getClip(dropEndPosition);

                if (firstNotReplacedClip)
                {
                    // Remove until the clip BEFORE firstNotReplacedClip
                    ASSERT(firstNotReplacedClip->getLeftPts() == dropEndPosition)(firstNotReplacedClip)(dropEndPosition);

                    to = drop.track->getPreviousClip(firstNotReplacedClip);
                    ASSERT(to->getRightPts() == dropEndPosition)(to)(dropEndPosition);
                }
                // else: Remove until end of track
            }
            // else: Clips are added 'beyond' the current track length

            model::Clips::const_iterator it = drop.track->getClips().begin();
            while (it != drop.track->getClips().end() && *it != from)
            { 
                ++it; 
            }
            while (it != drop.track->getClips().end() && *it != to) 
            {
                removedClips.push_back(*it);
                ++it;
            }
            if (it != drop.track->getClips().end())
            {
                removePosition = *it;
            }

            //      ================== ADD ===============  ================ REMOVE ================
            newMove(drop.track, removePosition, drop.clips, drop.track, removePosition, removedClips);
        }

        LOG_DEBUG << "STEP 5: Merge all consecutive white space";

        LOG_DEBUG << "STEP 6. Ensure that links are maintained.";
        replaceLinks(mConversion);
    }
    else
    {
        // The original behaviour (only used for deleting selected clips which should really be a separate command)
        // @todo thus
        BOOST_FOREACH( model::MoveParameterPtr move, mParams )
        {
            if (!redo)
            {
                mParamsUndo.push_back(move->make_inverted());
            }
            doMove(move);
        }
    }

    // @todo reverse mParamsUndo the first time

    return true;
}

bool AClipEdit::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( model::MoveParameterPtr move, mParamsUndo )
    {
        doMove(move);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AClipEdit::split(model::TrackPtr track, pts position, ReplacementMap* conversionmap)
{
    model::ClipPtr clip = track->getClip(position);
    if (clip)
    {
        // If there is a clip at the given position, it might need to be split
        position -= clip->getLeftPts();
        if (position != 0)
        {
            // If there is already a cut at the given position, nothing is changed.
            ASSERT(position < clip->getNumberOfFrames())(position)(clip->getNumberOfFrames());
            model::ClipPtr left = make_cloned<model::Clip>(clip);
            model::ClipPtr right = make_cloned<model::Clip>(clip);
            left->adjustEnd(position);
            right->adjustBegin(position);
            model::Clips replacements = boost::assign::list_of(left)(right);
            replaceClip(clip, replacements, conversionmap);
        }
    }
    else
    {
        // @todo insert emptyclips to ensure that the clip is pasted at the correct position
    }
}

void AClipEdit::replaceClip(model::ClipPtr original, model::Clips replacements, ReplacementMap* conversionmap)
{
    model::TrackPtr track = original->getTrack();
    model::ClipPtr position = track->getNextClip(original);
    model::Clips originallist = boost::assign::list_of(original);

    if (conversionmap)
    {
        ASSERT(conversionmap->find(original) == conversionmap->end());
        (*conversionmap)[ original ] = replacements;
    }

    //      ============= ADD ===========  ========== REMOVE ===========
    newMove(track, position, replacements, track, position, originallist);
}

void AClipEdit::replaceLinks(ReplacementMap& conversionmap)
{
    // For all replaced clips, ensure that the linked clip is also replaced,
    // at least with just a plain clone of the original link. This is needed to
    // avoid having these links 'dangling' after removal.
    BOOST_FOREACH( ReplacementMap::value_type link, conversionmap )
    {
        model::ClipPtr original = link.first;
        model::ClipPtr originallink = original->getLink();
        if (conversionmap.find(originallink) == conversionmap.end())
        {
            // If the original link wasn't already cut, then ensure that
            // a clone of that link is added to the list of possibly 
            // to be linked clips.
            model::ClipPtr clone = make_cloned<model::Clip>(originallink);
            clone->setLink(model::ClipPtr()); // The clone is linked to nothing, since linking is done below.
            replaceClip(originallink, boost::assign::list_of(clone), &conversionmap);
        }
    }

    // At this point all clips AND their original links (or their clones)
    // are part of the replacement map. Now the replacements for a clip
    // are linked to the replacements of its link.
    BOOST_FOREACH( ReplacementMap::value_type link, conversionmap )
    {
        model::ClipPtr clip1 = link.first;
        model::Clips new1 = link.second;
        model::Clips::iterator it1 = new1.begin();

        model::ClipPtr clip2 = clip1->getLink();
        model::Clips new2 = conversionmap[clip2];
        model::Clips::iterator it2 = new2.begin();

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
            (*it1)->setLink(model::ClipPtr());
            ++it1;
        }
        while ( it2 != new2.end() )
        {
            (*it2)->setLink(model::ClipPtr());
            ++it2;
        }
    }
}

void AClipEdit::newMove(model::TrackPtr addTrack, model::ClipPtr addPosition, model::Clips addClips, model::TrackPtr removeTrack, model::ClipPtr removePosition, model::Clips removeClips)
{
    model::MoveParameterPtr move = boost::make_shared<model::MoveParameter>(addTrack, addPosition, addClips, removeTrack, removePosition, removeClips);
    mParams.push_back(move);
    mParamsUndo.push_front(move->make_inverted()); // Must be executed in reverse order
    doMove(move);
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

std::ostream& operator<<( std::ostream& os, const AClipEdit::PasteInfo& obj )
{
    os << &obj << '|' << obj.track << '|' << obj.position << '|' << obj.clips;
    return os;
}

} // namespace