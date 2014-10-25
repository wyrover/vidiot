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

#include "TimelineDropTarget.h"

#include "State.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////////

TimelineDropTarget::TimelineDropTarget(Timeline* timeline)
    : Part(timeline)
    , wxDropTarget()
    , mOk(false)
    , mFormat(boost::none)
{

    wxDataObjectComposite* composite = new wxDataObjectComposite();
    composite->Add(new ProjectViewDataObject(), true);
    // todo composite->Add(new wxFileDataObject());
    SetDataObject(composite);
}

TimelineDropTarget::~TimelineDropTarget()
{
}

wxDragResult TimelineDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    ASSERT(mFormat);
    GetData(); // Required to initialize the received data object properly
    wxDataObjectComposite* composite = static_cast<wxDataObjectComposite *>(GetDataObject());
    wxString formatId = mFormat->GetId();
    wxDataFormatId formatType = static_cast<wxDataFormatId>(mFormat->GetType());
    if (formatId == ProjectViewDataObject::sFormat)
    {
        ProjectViewDataObject* object = dynamic_cast<ProjectViewDataObject*>(composite->GetObject(*mFormat));
    }
    else
    {
        ASSERT_EQUALS(formatType, wxDF_FILENAME);
        wxFileDataObject* object = static_cast<wxFileDataObject *>(composite->GetObject(*mFormat));
        //... use dataobj->GetFilenames() ...                
        // todo format for clips from timeline (new dataobject)
    }
    return def;
};
wxDragResult TimelineDropTarget::OnEnter(wxCoord x, wxCoord y, wxDragResult def)
{
    GetData(); // Required to initialize the received data object properly
    wxDataObjectComposite* composite = static_cast<wxDataObjectComposite *>(GetDataObject());
    mFormat.reset(composite->GetReceivedFormat());

    getMouse().dragMove(wxPoint(x, y));
    model::NodePtrs nodes = ProjectViewDropSource::get().getData().getAssets();
    mOk = true;
    for ( model::NodePtr node : nodes )
    {
        if (!node->isA<model::File>())
        {
            mOk = false;
            break;
        }
    }
    if (mOk)
    {
        ProjectViewDropSource::get().setFeedback(false);
        getKeyboard().update(state::EvKey(wxGetMouseState(),-1)); // To ensure that key events are 'seen' during the drag (timeline itself does not receive keyboard/mouse events)
        getStateMachine().process_event(state::EvDragEnter());
        return wxDragMove;
    }
    return wxDragNone;
}
wxDragResult TimelineDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    getKeyboard().update(state::EvKey(wxGetMouseState(), -1)); // To ensure that key events are 'seen' during the drag (timeline itself does not receive keyboard/mouse events)
    getMouse().dragMove(wxPoint(x,y));
    if (mOk)
    {
        // Accepted: key events not sent during dragging. Therefore the keyboard shortcuts for disabling snapping don't work when dragging from the project view.
        getStateMachine().process_event(state::EvDragMove());
        return wxDragMove;
    }
    return wxDragNone;
}
bool TimelineDropTarget::OnDrop(wxCoord x, wxCoord y)
{
    getMouse().dragMove(wxPoint(x,y));
    getStateMachine().process_event(state::EvDragDrop());
    return true;
}
void TimelineDropTarget::OnLeave()
{
    ProjectViewDropSource::get().setFeedback(true);
    getStateMachine().process_event(state::EvDragEnd());
    mFormat.reset();
}

}} // namespace
