// Copyright 2014-2016 Eric Raijmakers.
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

#pragma once

#include "Part.h"

namespace gui { namespace timeline {

class TimelineDropTarget
    : public Part
    , public wxDropTarget
{
public:
    
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    TimelineDropTarget(Timeline* timeline);
    ~TimelineDropTarget();
    
    //////////////////////////////////////////////////////////////////////////
    // wxDropTarget interface 
    //////////////////////////////////////////////////////////////////////////

    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
    wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def) override;
    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;
    bool OnDrop(wxCoord x, wxCoord y) override;
    void OnLeave() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::TrackPtr getVideo();
    model::TrackPtr getAudio();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::optional<wxDataFormat> mFormat;
    model::NodePtrs mNodes;
    model::TrackPtr mVideo;
    model::TrackPtr mAudio;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    bool validDataDragged() const;

};

}} // namespace
