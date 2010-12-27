#include "CreateVideoTrack.h"

#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "Timeline.h"
#include "Sequence.h"
#include "VideoTrack.h"

namespace gui { namespace timeline { namespace command {

CreateVideoTrack::CreateVideoTrack(gui::timeline::Timeline& timeline)
:   ATimelineCommand(timeline)
,   mNewTrack()
{
    VAR_INFO(this);
    mCommandName = _("Add track"); 
}

CreateVideoTrack::~CreateVideoTrack()
{
}
//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////


bool CreateVideoTrack::Do()
{
    VAR_INFO(this);
    if (!mNewTrack)
    {
        mNewTrack = boost::make_shared<model::VideoTrack>();
    }
    getTimeline().getSequence()->addVideoTracks(boost::assign::list_of(mNewTrack));
    return true;
}

bool CreateVideoTrack::Undo()
{
    VAR_INFO(this);
    getTimeline().getSequence()->removeVideoTracks(boost::assign::list_of(mNewTrack));
    return true;
}

}}} // namespace
