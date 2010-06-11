#include "ProjectCommandCreateAudioTrack.h"
#include "UtilLog.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

ProjectCommandCreateAudioTrack::ProjectCommandCreateAudioTrack(model::SequencePtr sequence)
:   ProjectCommand()
,   mSequence(sequence)
,   mNewTrack()
{
    VAR_INFO(this)(mSequence);
    mCommandName = _("Add track"); 
}

ProjectCommandCreateAudioTrack::~ProjectCommandCreateAudioTrack()
{
}

bool ProjectCommandCreateAudioTrack::Do()
{
    if (!mNewTrack)
    {
        mNewTrack = boost::make_shared<model::AudioTrack>();
    }
    mSequence->addAudioTrack(mNewTrack);
    return true;
}

bool ProjectCommandCreateAudioTrack::Undo()
{
    mSequence->removeAudioTrack(mNewTrack);
    return true;
}
