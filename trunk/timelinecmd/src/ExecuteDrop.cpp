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

    ReplacementMap linkmapper;

    LOG_DEBUG << "STEP 1: Replace all drags with EmptyClips";
    BOOST_FOREACH( model::ClipPtr clip, mDrags )
    {
        replaceClip(clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getNumberOfFrames())));
    }

    LOG_DEBUG << "STEP 2: Execute the drops AND fill replacement map";
    BOOST_FOREACH( Drop drop, mDrops )
    {
        ASSERT(drop.position >= 0)(drop.position);
        ASSERT(drop.track);
        ASSERT(drop.clips.size() != 0);
        VAR_DEBUG(drop.position)(drop.track)(drop.clips);

        // Determine size and end pts of dropped clips
        pts droppedSize = model::Clip::getCombinedLength(drop.clips);
        pts dropEndPosition = drop.position + droppedSize;

        // Ensure that the track has cuts at the begin and the end of the dropped clips
        split(drop.track, drop.position,   &linkmapper);
        split(drop.track, dropEndPosition, &linkmapper);

        // Determine the clips to be replaced.
        // Done AFTER the splitting above, since that requires clip addition/removal.
        AClipEdit::ClipsWithPosition remove = findClips(drop.track, drop.position, dropEndPosition);

        if (drop.position > drop.track->getNumberOfFrames())
        {
            // Drop is beyond track length. Add an empty clip to have it a at the desired position (instead of directly after last clip).
            ASSERT(!remove.second)(remove.second); // The position of the drop should be a null ptr, since the drop is at the end of the track
            newMove(drop.track, remove.second, boost::assign::list_of(boost::make_shared<model::EmptyClip>(drop.position - drop.track->getNumberOfFrames())) );
        }

        //      ================ ADD ===============  =============== REMOVE ================
        newMove(drop.track, remove.second, drop.clips, drop.track, remove.second, remove.first);
    }

    LOG_DEBUG << "STEP 3: Ensure that links are maintained.";
    replaceLinks(linkmapper);
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
