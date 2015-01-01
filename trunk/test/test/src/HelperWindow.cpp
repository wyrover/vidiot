// Copyright 2013-2015 Eric Raijmakers.
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

#include "Test.h"

namespace test {

void WindowTriggerMenu(int id)
{
    WindowTriggerMenu(gui::Window::get(), id);
}

void WindowTriggerMenu(wxWindow& window, int id)
{
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
    WaitForIdle();
}

void WindowCheckMenu(int id, bool checked)
{
    WindowCheckMenu(gui::Window::get(), id, checked);
}

void WindowCheckMenu(wxFrame& window, int id, bool checked)
{
     // Set with the 'other' value. Required to ensure that the
    // correct value is received when triggering the event,
    // since the event causes a 'toggle'.
    window.GetMenuBar()->Check(id,!checked);
    window.ProcessCommand(id);
    WaitForIdle();
}

model::FolderPtr WindowCreateProject()
{
    WaitForIdle();
    WindowTriggerMenu(wxID_NEW);
    WaitForIdle();
    return getRoot();
}

wxMenu* getSequenceMenu()
{
    return gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex);
}

void Undo(int steps)
{
    LOG_DEBUG;
    while (steps > 0)
    {
        WindowTriggerMenu(wxID_UNDO);
        LogHistory();
        steps--;
    }
}

void Redo(int steps)
{
    LOG_DEBUG;
    while (steps > 0)
    {
        WindowTriggerMenu(wxID_REDO);
        LogHistory();
        steps--;
    }
}

wxCommand* getCurrentCommand()
{
    return gui::Window::get().GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->GetCurrentCommand();
}

void BREAK()
{
#if (defined _MSC_VER) || (defined __BORLANDC__)
            __asm { int 3 };
#elif (defined __GNUC__) && (defined _DEBUG)
        __asm ("int $0x3");
#endif
    gui::Window::get().Raise();
}

void LogHistory()
{
    LOG_DEBUG;
    wxCommandProcessor* proc = gui::Window::get().GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor();
    wxList& cmds = proc->GetCommands();
    wxList::compatibility_iterator it = cmds.GetFirst();
    while (it != cmds.GetLast())
    {
        wxObject* command = it->GetData();
        std::string type = typeid(*command).name();
        VAR_DEBUG(command)(type);
        it = it->GetNext();
    }
    wxCommand* current = proc->GetCurrentCommand();
    VAR_DEBUG(current);
};

} // namespace
