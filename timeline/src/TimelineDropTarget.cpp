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

#include "TimelineDropTarget.h"

#include "EventDrag.h"
#include "FileAnalyzer.h"
#include "ProjectViewDropSource.h"
#include "State.h"
#include "TrackCreator.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////////

TimelineDropTarget::TimelineDropTarget(Timeline* timeline)
    : Part(timeline)
    , wxDropTarget()
    , mFormat(boost::none)
    , mNodes()
{

    wxDataObjectComposite* composite = new wxDataObjectComposite();
    composite->Add(new ProjectViewDataObject());
    // NOT: composite->Add(new TimelineDataObject(), true); -- Never via wxWidgets dnd interfaces, but handled directly
    composite->Add(new wxFileDataObject());
    SetDataObject(composite);
}

TimelineDropTarget::~TimelineDropTarget()
{
}

wxDragResult TimelineDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    return def;
};

wxDragResult TimelineDropTarget::OnEnter(wxCoord x, wxCoord y, wxDragResult def)
{
    GetData(); // Required to initialize the received data object properly
    wxDataObjectComposite* composite = static_cast<wxDataObjectComposite *>(GetDataObject());
    mFormat.reset(composite->GetReceivedFormat());
	mNodes.clear();

    if (static_cast<wxDataFormatId>(mFormat->GetType()) == wxDF_FILENAME)
    {
        wxFileDataObject* object = static_cast<wxFileDataObject*>(composite->GetObject(*mFormat));
        ASSERT_NONZERO(object);
        boost::shared_ptr<model::FileAnalyzer> analyzer = boost::make_shared<model::FileAnalyzer>(object->GetFilenames());
        if (analyzer->isProjectOnly())
        {
            return wxDragNone;
        }
        else if (analyzer->checkIfOkForPasteOrDrop())
        {
            mNodes = analyzer->getNodes();
        }
    }
    else if (mFormat->GetId() == ProjectViewDataObject::sFormat)
    {
        ProjectViewDataObject* object = dynamic_cast<ProjectViewDataObject*>(composite->GetObject(*mFormat));
        ASSERT_NONZERO(object);
		if (object->checkIfOkForPasteOrDrop())
		{
			mNodes = object->getNodes();
		}
    }

    getMouse().dragMove(wxPoint(x, y));

    ::cmd::TrackCreator c(mNodes);
    mVideo = c.getVideoTrack();
    mAudio = c.getAudioTrack();

    if (mVideo->getLength() > 0 &&
        mAudio->getLength() > 0)
    {
        ProjectViewDropSource::get().setFeedback(false);
        wxMouseState mouseState = wxGetMouseState();
        state::EvKey keyEvent(mouseState,-1);
        getKeyboard().update(keyEvent); // To ensure that key events are 'seen' during the drag (timeline itself does not receive keyboard/mouse events)
        getStateMachine().process_event(state::EvDragEnter());
        return wxDragCopy;
    }
    else
    {
        mVideo.reset();
        mAudio.reset();
    }
    return wxDragNone;
}

wxDragResult TimelineDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    wxMouseState mouseState = wxGetMouseState();
    getKeyboard().update(state::EvKey(mouseState, -1)); // To ensure that key events are 'seen' during the drag (timeline itself does not receive keyboard/mouse events)
    getMouse().dragMove(wxPoint(x,y));
    if (validDataDragged())
    {
        // Accepted: key events not sent during dragging. Therefore the keyboard shortcuts for disabling snapping don't work when dragging from the project view.
        getStateMachine().process_event(state::EvDragMove());
        return wxDragCopy;
    }
    return wxDragNone;
}

bool TimelineDropTarget::OnDrop(wxCoord x, wxCoord y)
{
    getMouse().dragMove(wxPoint(x,y));
    getStateMachine().process_event(state::EvDragDrop());
    mNodes.clear();
    mVideo.reset();
    mAudio.reset();
    return true;
}

void TimelineDropTarget::OnLeave()
{
    ProjectViewDropSource::get().setFeedback(true);
    getStateMachine().process_event(state::EvDragEnd());
    mFormat.reset();
    mNodes.clear();
    mVideo.reset();
    mAudio.reset();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::TrackPtr TimelineDropTarget::getVideo()
{
    return mVideo;
}

model::TrackPtr TimelineDropTarget::getAudio()
{
    return mAudio;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool TimelineDropTarget::validDataDragged() const
{
    return !mNodes.empty();
}

}} // namespace
