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

#include "TimelineClipboard.h"

#include "Cursor.h"
#include "EventClipboard.h"
#include "ExecuteDrop.h"
#include "FileAnalyzer.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ProjectViewDataObject.h"
#include "Sequence.h"
#include "Selection.h"
#include "State.h"
#include "Timeline.h"
#include "TimelineDataObject.h"
#include "Track.h"
#include "TrackCreator.h"
#include "UtilLog.h"
#include "Window.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TimelineClipboard::TimelineClipboard(Timeline* timeline)
    : Part(timeline)
{
    VAR_DEBUG(this);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &TimelineClipboard::onCutFromMainMenu, this, wxID_CUT);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &TimelineClipboard::onCopyFromMainMenu, this, wxID_COPY);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &TimelineClipboard::onPasteFromMainMenu, this, wxID_PASTE);
}

TimelineClipboard::~TimelineClipboard()
{
    VAR_DEBUG(this);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &TimelineClipboard::onCutFromMainMenu, this, wxID_CUT);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &TimelineClipboard::onCopyFromMainMenu, this, wxID_COPY);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &TimelineClipboard::onPasteFromMainMenu, this, wxID_PASTE);
}

//////////////////////////////////////////////////////////////////////////
// MAIN WINDOW EDIT MENU
//////////////////////////////////////////////////////////////////////////

void TimelineClipboard::onCutFromMainMenu(wxCommandEvent& event)
{
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        getStateMachine().process_event(timeline::state::EvCut());
    }
}

void TimelineClipboard::onCopyFromMainMenu(wxCommandEvent& event)
{
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        getStateMachine().process_event(timeline::state::EvCopy());
    }
}

void TimelineClipboard::onPasteFromMainMenu(wxCommandEvent& event)
{
    // only if one node is selected and that node is a folder or no node is selected (root node)
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        getStateMachine().process_event(timeline::state::EvPaste());
    }
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void TimelineClipboard::onCut()
{
    LOG_INFO;
    TimelineDataObject* dataObject = new TimelineDataObject(getSequence());
    if (dataObject->storeInClipboard())
    {
		getSelection().deleteClips();
    }
}

void TimelineClipboard::onCopy()
{
    LOG_INFO;
    TimelineDataObject* dataObject = new TimelineDataObject(getSequence());
    dataObject->storeInClipboard();
}

void TimelineClipboard::onPaste(bool atCursor)
{
    LOG_INFO;
	model::NodePtrs nodes;
    if (wxTheClipboard->Open())
    {
        pts dropPosition = atCursor ? getCursor().getLogicalPosition() : getZoom().pixelsToPts(getMouse().getRightUpPosition().x);
        if (wxTheClipboard->IsSupported(TimelineDataObject::sFormat))
        {
            TimelineDataObject data;
            wxTheClipboard->GetData(data);
            wxTheClipboard->Close();
			if (data.checkIfOkForPasteOrDrop())
			{
				command::ExecuteDrop* command = new command::ExecuteDrop(getSequence(), true);
				command->onDrop(data.getDrops(getSequence(), dropPosition));
				command->submit();
			}
        }
        else
		{
			if (wxTheClipboard->IsSupported(wxDataFormat(wxDF_FILENAME)))
			{
				wxFileDataObject data;
				wxTheClipboard->GetData(data);
				wxTheClipboard->Close();
				boost::shared_ptr<model::FileAnalyzer> analyzer = boost::make_shared<model::FileAnalyzer>(data.GetFilenames());
				if (analyzer->checkIfOkForPasteOrDrop())
				{
					nodes = analyzer->getNodes();
				}
			}
			else if (wxTheClipboard->IsSupported(wxDataFormat(ProjectViewDataObject::sFormat)))
			{
				ProjectViewDataObject data;
				wxTheClipboard->GetData(data);
				wxTheClipboard->Close();
				if (data.checkIfOkForPasteOrDrop())
				{
					nodes = data.getNodes();
				}
			}
			else
			{
				wxTheClipboard->Close();
			}
			if (!nodes.empty())
			{
				::command::TrackCreator c(nodes);
				model::TrackPtr video = c.getVideoTrack();
				model::TrackPtr audio = c.getAudioTrack();
				command::Drops drops;
				command::Drop videoDrop;
				videoDrop.clips = video->getClips();
				videoDrop.position = dropPosition;
				videoDrop.track = getSequence()->getVideoTrack(0);
				command::Drop audioDrop;
				audioDrop.clips = audio->getClips();
				audioDrop.position = dropPosition;
				audioDrop.track = getSequence()->getAudioTrack(0);
				drops.push_back(videoDrop);
				drops.push_back(audioDrop);
				command::ExecuteDrop* command = new command::ExecuteDrop(getSequence(), true);
				command->onDrop(drops);
				command->submit();
			}
		}
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool TimelineClipboard::canPaste()
{
	bool result = false;
	if (wxTheClipboard->Open())
	{
		if ((wxTheClipboard->IsSupported(TimelineDataObject::sFormat)) ||
		    (wxTheClipboard->IsSupported(wxDataFormat(wxDF_FILENAME))) ||
		    (wxTheClipboard->IsSupported(wxDataFormat(ProjectViewDataObject::sFormat))))
		{
			result = true;
		}
		wxTheClipboard->Close();
	}
	return result;
}


//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool TimelineClipboard::hasKeyboardFocus() const
{
    if (getTimeline().isActive())
    {
        wxWindow* focused = wxWindow::FindFocus();
        if (focused != 0)
        {
            if (dynamic_cast<timeline::Timeline*>(focused) != 0)
            {
                return true;
            }
        }
    }
    return false;
}

}} // namespace