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

#ifndef REMOVE_EMPTY_TRACKS_H
#define REMOVE_EMPTY_TRACKS_H

#include "ATimelineCommand.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline { namespace command {

class RemoveEmptyTracks
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit RemoveEmptyTracks (model::SequencePtr sequence);
    virtual ~RemoveEmptyTracks ();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS FROM SEQUENCE
    //////////////////////////////////////////////////////////////////////////

    void onVideoTracksRemoved( model::EventRemoveVideoTracks& event );
    void onAudioTracksRemoved( model::EventRemoveAudioTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;
    std::list< model::TrackChange > mVideoUndo;
    std::list< model::TrackChange > mAudioUndo;

};

}}} // namespace

#endif // REMOVE_EMPTY_TRACKS_H