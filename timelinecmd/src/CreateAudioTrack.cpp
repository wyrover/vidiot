#include "CreateAudioTrack.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "UtilLog.h"
#include "AudioTrack.h"
#include "Timeline.h"
#include "Sequence.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CreateAudioTrack::CreateAudioTrack(model::SequencePtr sequence)
    :   ATimelineCommand(sequence)
    ,   mNewTrack()
{
    VAR_INFO(this);
    mCommandName = _("Add track"); 
}

CreateAudioTrack::~CreateAudioTrack()
{
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool CreateAudioTrack::Do()
{
    if (!mNewTrack)
    {
        mNewTrack = boost::make_shared<model::AudioTrack>();
    }
    getTimeline().getSequence()->addAudioTracks(boost::assign::list_of(mNewTrack));
    return true;
}

bool CreateAudioTrack::Undo()
{
    getTimeline().getSequence()->removeAudioTracks(boost::assign::list_of(mNewTrack));
    return true;
}

}}} // namespace
