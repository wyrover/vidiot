// Copyright 2014 Eric Raijmakers.
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

#ifndef TIMELINE_DROP_TARGET_H
#define TIMELINE_DROP_TARGET_H

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

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mOk;
    boost::optional<wxDataFormat> mFormat;
};

}} // namespace

#endif
