#include "DeleteSelectedClips.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "Track.h"
#include "Clip.h"
#include "Sequence.h"
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
    deleteSelectedClips(getTimeline().getSequence()->getVideoTracks());
    deleteSelectedClips(getTimeline().getSequence()->getAudioTracks());
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DeleteSelectedClips::deleteSelectedClips(model::Tracks tracks)
{
    BOOST_FOREACH( model::TrackPtr track, tracks )
    {
        long nRemovedFrames = 0;
        model::Clips removed;
        BOOST_FOREACH( model::ClipPtr clip, track->getClips() )
        {
            if (clip->getSelected())
            {
                removed.push_back(clip);
                nRemovedFrames += clip->getNumberOfFrames();
            }
            else
            {
                if (!removed.empty())
                {
                    // First not selected clip after some selected clips found. Replace these clips with an empty one.
                    newMove(track, clip, boost::assign::list_of(boost::make_shared<model::EmptyClip>(nRemovedFrames)), track, clip, removed );
                    removed.clear();    // Prepare for possible next region
                    nRemovedFrames = 0; // Prepare for possible next region
                }
            }
        }
        if (!removed.empty())
        {
            // The last clips of the track are removed.
            newMove(track, model::ClipPtr(), boost::assign::list_of(boost::make_shared<model::EmptyClip>(nRemovedFrames)), track, model::ClipPtr(), removed );
        }
    }
}

}}} // namespace
