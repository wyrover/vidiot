#include "DeleteSelectedClips.h"

#include <set>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "Track.h"
#include "Clip.h"
#include "Sequence.h"
#include "Transition.h"
#include "Timeline.h"
#include "EmptyClip.h"

namespace gui { namespace timeline { namespace command {

DeleteSelectedClips::DeleteSelectedClips(gui::timeline::Timeline& timeline)
    :   AClipEdit(timeline)
{
    VAR_INFO(this);
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
    ReplacementMap linkmapper;
    std::set<model::TransitionPtr> transitionsToBeRemoved;

    BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
    {
        model::IClips clips = track->getClips(); // Make copy of list. In the loop (iteration) the original list (track->getClips()) is changed thus cannot be used for iteration.

        BOOST_FOREACH( model::IClipPtr clip, clips )
        {
            if (clip->getSelected())
            {
                model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
                if (transition)
                {
                    transitionsToBeRemoved.insert(transition);
                }
                else
                {
                    // Also delete transitions that overlap with this clip
                    model::TransitionPtr prevTransition = boost::dynamic_pointer_cast<model::Transition>(track->getPreviousClip(clip));
                    model::TransitionPtr nextTransition = boost::dynamic_pointer_cast<model::Transition>(track->getNextClip(clip));

                    if (prevTransition && prevTransition->getRight() > 0)
                    {
                        transitionsToBeRemoved.insert(prevTransition);
                    }
                    if (nextTransition && nextTransition->getLeft() > 0)
                    {
                        transitionsToBeRemoved.insert(nextTransition);
                    }

                    replaceClip(clip,boost::assign::list_of(boost::make_shared<model::EmptyClip>(clip->getLength())),&linkmapper);
                }
            }
        }
    }

    // Replace transitions. Is done in a second step to simplify the various possible options (transitions with and without left and right clips)
    // and to avoid problems with removing the left clip first, without the transition or removing the transition before the right clip
    BOOST_FOREACH( model::TransitionPtr transition, transitionsToBeRemoved )
    {
        removeTransition(transition, linkmapper);
    }

    replaceLinks(linkmapper);
}

}}} // namespace
