#include "TimelineCreateAudioTrack.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "UtilLog.h"

namespace command {

TimelineCreateAudioTrack::TimelineCreateAudioTrack(model::SequencePtr sequence)
:   TimelineCommand()
,   mSequence(sequence)
,   mNewTrack()
{
    VAR_INFO(this)(mSequence);
    mCommandName = _("Add track"); 
}

TimelineCreateAudioTrack::~TimelineCreateAudioTrack()
{
}

bool TimelineCreateAudioTrack::Do()
{
    if (!mNewTrack)
    {
        mNewTrack = boost::make_shared<model::AudioTrack>();
    }
    mSequence->addAudioTrack(mNewTrack);
    return true;
}

bool TimelineCreateAudioTrack::Undo()
{
    mSequence->removeAudioTrack(mNewTrack);
    return true;
}

} // namespace