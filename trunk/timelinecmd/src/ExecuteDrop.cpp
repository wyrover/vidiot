#include "ExecuteDrop.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "Track.h"
#include "Clip.h"
#include "EmptyClip.h"
#include "Transition.h"
#include "Timeline.h"
#include "Sequence.h"

namespace gui { namespace timeline { namespace command {

// static
const pts ExecuteDrop::sNoShift = -1;

ExecuteDrop::ExecuteDrop(gui::timeline::Timeline& timeline, model::IClips drags, Drops drops, pts shiftPosition, pts shiftSize)
:   AClipEdit(timeline)
,   mDrags(drags)
,   mDrops(drops)
,   mShiftPosition(shiftPosition)
,   mShiftSize(shiftSize)
{
    VAR_INFO(this)(drags)(drops)(shiftPosition)(shiftSize);
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
    BOOST_FOREACH( model::IClipPtr clip, mDrags )
    {
        // todo if the clips to the left or right of a transition are removed here, the transition must also be removed
        if (clip->isA<model::Transition>())
        {
            // Transitions are simply removed
            removeClip(clip);
        }
        else
        {
            replaceClip(clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getLength())));
        }
    }

    if (mShiftPosition >= 0)
    {
        LOG_DEBUG << "STEP 2: Apply shift";
        BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(mShiftPosition);
            newMove(track, clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(mShiftSize)));
        }
    }
    else
    {
        LOG_DEBUG << "STEP 2: Apply shift (none)";
    }

    LOG_DEBUG << "STEP 3: Execute the drops AND fill replacement map";
    BOOST_FOREACH( Drop drop, mDrops )
    {
        ASSERT(drop.position >= 0)(drop.position);
        ASSERT(drop.track);
        ASSERT(drop.clips.size() != 0);
        VAR_DEBUG(drop.position)(drop.track)(drop.clips);

        // Determine size and end pts of dropped clips
        pts droppedSize = model::Track::getCombinedLength(drop.clips);
        pts dropEndPosition = drop.position + droppedSize;

        // Ensure that the track has cuts at the begin and the end of the dropped clips
        split(drop.track, drop.position,   &linkmapper);
        split(drop.track, dropEndPosition, &linkmapper);

        // Determine the clips to be replaced.
        // Done AFTER the splitting above, since that requires clip addition/removal.
        // Furthermore, note that the split calls above ensure that any transitions at
        // the begin and end of the insertion are removed.
        AClipEdit::ClipsWithPosition remove = findClips(drop.track, drop.position, dropEndPosition);

        if (drop.position > drop.track->getLength())
        {
            // Drop is beyond track length. Add an empty clip to have it a at the desired position (instead of directly after last clip).
            ASSERT(!remove.second)(remove.second); // The position of the drop should be a null ptr, since the drop is at the end of the track
            newMove(drop.track, remove.second, boost::assign::list_of(boost::make_shared<model::EmptyClip>(drop.position - drop.track->getLength())) );
        }

        //      ================ ADD ===============  =============== REMOVE ================
        newMove(drop.track, remove.second, drop.clips, drop.track, remove.second, remove.first);
    }

    LOG_DEBUG << "STEP 4: Ensure that links are maintained.";
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
