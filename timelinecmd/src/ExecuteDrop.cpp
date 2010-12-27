#include "ExecuteDrop.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "Track.h"
#include "Clip.h"
#include "EmptyClip.h"

namespace gui { namespace timeline { namespace command {

ExecuteDrop::ExecuteDrop(gui::timeline::Timeline& timeline, model::Clips drags, Drops drops)
:   AClipEdit(timeline)
,   mDrags(drags)
,   mDrops(drops)
{
    VAR_INFO(this)(drags)(drops);
    mCommandName = _("Move clips");
}

ExecuteDrop::~ExecuteDrop()
{
}

void ExecuteDrop::initialize()
{
    VAR_INFO(this);

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
    BOOST_FOREACH( Drop drop, mDrops )
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ExecuteDrop::Drop& obj )
{
    os << &obj << '|' << obj.track << '|' << obj.position << '|' << obj.clips;
    return os;
}

}}} // namespace
