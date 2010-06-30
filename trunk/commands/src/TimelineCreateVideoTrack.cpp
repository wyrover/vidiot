#include "ProjectCommandCreateVideoTrack.h"
#include "UtilLog.h"
#include <boost/make_shared.hpp>

ProjectCommandCreateVideoTrack::ProjectCommandCreateVideoTrack(model::SequencePtr sequence)
:   ProjectCommand()
,   mSequence(sequence)
,   mNewTrack()
{
    VAR_INFO(this)(mSequence);
    mCommandName = _("Add track"); 
}

ProjectCommandCreateVideoTrack::~ProjectCommandCreateVideoTrack()
{
}

bool ProjectCommandCreateVideoTrack::Do()
{
    VAR_INFO(this);
    if (!mNewTrack)
    {
        mNewTrack = boost::make_shared<model::VideoTrack>();
    }
    mSequence->addVideoTrack(mNewTrack);
    return true;
}

bool ProjectCommandCreateVideoTrack::Undo()
{
    VAR_INFO(this);
    mSequence->removeVideoTrack(mNewTrack);
    return true;
}
