// Copyright 2013-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "CreateAudioTrack.h"

#include "AudioTrack.h"
#include "Timeline.h"
#include "Sequence.h"

namespace gui { namespace timeline { namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CreateAudioTrack::CreateAudioTrack(const model::SequencePtr& sequence)
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
    getTimeline().getSequence()->addAudioTracks({ mNewTrack });
    return true;
}

bool CreateAudioTrack::Undo()
{
    getTimeline().getSequence()->removeAudioTracks({ mNewTrack });
    return true;
}

}}} // namespace