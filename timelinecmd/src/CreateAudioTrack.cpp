#include "TimelineCreateAudioTrack.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "UtilLog.h"

namespace command {

TimelineCreateAudioTrack::TimelineCreateAudioTrack(gui::timeline::Timeline& timeline, model::SequencePtr sequence)
:   TimelineCommand(timeline)
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
    mSequence->addAudioTracks(boost::assign::list_of(mNewTrack));
    return true;
}

bool TimelineCreateAudioTrack::Undo()
{
    mSequence->removeAudioTracks(boost::assign::list_of(mNewTrack));
    return true;
}

} // namespace