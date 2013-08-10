// Copyright 2013 Eric Raijmakers.
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