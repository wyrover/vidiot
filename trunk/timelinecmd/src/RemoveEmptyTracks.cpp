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

#include "RemoveEmptyTracks.h"

#include "UtilLog.h"
#include "AudioTrack.h"
#include "Timeline.h"
#include "Sequence.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

RemoveEmptyTracks::RemoveEmptyTracks(model::SequencePtr sequence)
    :   ATimelineCommand(sequence)
    ,   mInitialized(false)
{
    VAR_INFO(this);
    mCommandName = _("Remove empty tracks");
}

RemoveEmptyTracks::~RemoveEmptyTracks()
{
}

//////////////////////////////////////////////////////////////////////////
// EVENTS FROM SEQUENCE
//////////////////////////////////////////////////////////////////////////

void RemoveEmptyTracks::onVideoTracksRemoved( model::EventRemoveVideoTracks& event )
{
    mVideoUndo.push_back(event.getValue().make_inverted());
    event.Skip();
}

void RemoveEmptyTracks::onAudioTracksRemoved( model::EventRemoveAudioTracks& event )
{
    mAudioUndo.push_back(event.getValue().make_inverted());
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool RemoveEmptyTracks::Do()
{
    getSequence()->Bind(model::EVENT_REMOVE_VIDEO_TRACK,    &RemoveEmptyTracks::onVideoTracksRemoved,  this);
    getSequence()->Bind(model::EVENT_REMOVE_AUDIO_TRACK,    &RemoveEmptyTracks::onAudioTracksRemoved,  this);

    model::Tracks videoTracks = getSequence()->getVideoTracks();
    BOOST_FOREACH( model::TrackPtr track, videoTracks )
    {
        if (track->isEmpty() && getSequence()->getVideoTracks().size() > 1) // Always one track must remain
        {
            getSequence()->removeVideoTracks(boost::assign::list_of(track));
        }
    }
    model::Tracks audioTracks = getSequence()->getAudioTracks();
    BOOST_FOREACH( model::TrackPtr track, audioTracks )
    {
        if (track->isEmpty() && getSequence()->getAudioTracks().size() > 1) // Always one track must remain
        {
            getSequence()->removeAudioTracks(boost::assign::list_of(track));
        }
    }
    getSequence()->Unbind(model::EVENT_REMOVE_VIDEO_TRACK,    &RemoveEmptyTracks::onVideoTracksRemoved,  this);
    getSequence()->Unbind(model::EVENT_REMOVE_AUDIO_TRACK,    &RemoveEmptyTracks::onAudioTracksRemoved,  this);
    return true;
}

bool RemoveEmptyTracks::Undo()
{
    BOOST_FOREACH( model::TrackChange change, mVideoUndo )
    {
        getSequence()->addVideoTracks(change.addedTracks,change.addPosition);
    }
    BOOST_FOREACH( model::TrackChange change, mAudioUndo )
    {
        getSequence()->addAudioTracks(change.addedTracks,change.addPosition);
    }
    mVideoUndo.clear();
    mAudioUndo.clear();
    return true;
}

}}} // namespace