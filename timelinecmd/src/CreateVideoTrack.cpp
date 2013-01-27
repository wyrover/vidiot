#include "CreateVideoTrack.h"

#include "UtilLog.h"
#include "Timeline.h"
#include "Sequence.h"
#include "VideoTrack.h"

namespace gui { namespace timeline { namespace command {

CreateVideoTrack::CreateVideoTrack(model::SequencePtr sequence)
:   ATimelineCommand(sequence)
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