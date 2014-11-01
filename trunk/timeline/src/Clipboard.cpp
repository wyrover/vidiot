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

#include "Clipboard.h"

#include "Timeline.h"
#include "UtilLog.h"
#include "Window.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Clipboard::Clipboard(Timeline* timeline)
    : Part(timeline)
{
    VAR_DEBUG(this);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &Clipboard::onCutFromMainMenu, this, wxID_CUT);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &Clipboard::onCopyFromMainMenu, this, wxID_COPY);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &Clipboard::onPasteFromMainMenu, this, wxID_PASTE);
}

Clipboard::~Clipboard()
{
    VAR_DEBUG(this);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &Clipboard::onCutFromMainMenu, this, wxID_CUT);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &Clipboard::onCopyFromMainMenu, this, wxID_COPY);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &Clipboard::onPasteFromMainMenu, this, wxID_PASTE);
}

//////////////////////////////////////////////////////////////////////////
// MAIN WINDOW EDIT MENU
//////////////////////////////////////////////////////////////////////////

void Clipboard::onCutFromMainMenu(wxCommandEvent& event)
{
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        onCut();
    }
}

void Clipboard::onCopyFromMainMenu(wxCommandEvent& event)
{
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        onCopy();
    }
}

void Clipboard::onPasteFromMainMenu(wxCommandEvent& event)
{
    // only if one node is selected and that node is a folder or no node is selected (root node)
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        onPaste();
    }

}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void Clipboard::onCut()
{
    LOG_INFO;
    // todo via state machine?
    // todo popup menu?
}

void Clipboard::onCopy()
{
    LOG_INFO;

}

void Clipboard::onPaste()
{
    LOG_INFO;

}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool Clipboard::hasKeyboardFocus() const
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