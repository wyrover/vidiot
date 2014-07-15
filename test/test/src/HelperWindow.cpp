// Copyright 2013,2014 Eric Raijmakers.
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

void triggerMenu(int id)
{
    triggerMenu(gui::Window::get(), id);
}

void triggerMenu(wxWindow& window, int id)
{
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
    waitForIdle();
}

void checkMenu(int id, bool checked)
{
    checkMenu(gui::Window::get(), id, checked);
}

void checkMenu(wxFrame& window, int id, bool checked)
{
     // Set with the 'other' value. Required to ensure that the
    // correct value is received when triggering the event,
    // since the event causes a 'toggle'.
    window.GetMenuBar()->Check(id,!checked);
    window.ProcessCommand(id);
    waitForIdle();
}

model::FolderPtr createProject()
{
    waitForIdle();
    triggerMenu(wxID_NEW);
    waitForIdle();
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
        triggerMenu(wxID_UNDO);
        logHistory();
        steps--;
    }
}

void Redo(int steps)
{
    LOG_DEBUG;
    while (steps > 0)
    {
        triggerMenu(wxID_REDO);
        logHistory();
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

void logHistory()
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

void LeftDown(bool wait)
{
    wxUIActionSimulator().MouseDown();
    if (wait) { waitForIdle(); }
}

void LeftUp(bool wait)
{
    wxUIActionSimulator().MouseUp();
    if (wait) { waitForIdle(); }
}

void RightDown(bool wait)
{
    wxUIActionSimulator().MouseDown(wxMOUSE_BTN_RIGHT);
    if (wait) { waitForIdle(); }
}

void RightUp(bool wait)
{
    wxUIActionSimulator().MouseUp(wxMOUSE_BTN_RIGHT);
    if (wait) { waitForIdle(); }
}

void ClickLeft(bool wait)
{
    wxUIActionSimulator().MouseClick(wxMOUSE_BTN_LEFT);
    if (wait) { waitForIdle(); }
}

void ClickRight(bool wait)
{
    wxUIActionSimulator().MouseClick(wxMOUSE_BTN_RIGHT);
    if (wait) { waitForIdle(); }
}

void ControlDown()
{
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    waitForIdle();
}

void ControlUp()
{
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    waitForIdle();
}

void ShiftDown()
{
    wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    waitForIdle();
}

void ShiftUp()
{
    wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void Type(int keycode, int modifiers)
{
    wxUIActionSimulator().Char(keycode,modifiers);
    waitForIdle();
}

void TypeN(int count, int keycode, int modifiers)
{
    for (int i = 0; i < count; ++i) { Type(keycode,modifiers); }
}

void MoveRight(pixel length)
{
    MoveOnScreen(wxGetMouseState().GetPosition() + wxPoint(length,0));
}

void MoveLeft(pixel length)
{
    MoveOnScreen(wxGetMouseState().GetPosition() + wxPoint(-length,0));
}

void MoveWithinWidget(wxPoint position, wxPoint origin)
{
    VAR_DEBUG(position)(origin);
    wxPoint absoluteposition = origin + position;
    MoveOnScreen(absoluteposition);
}

void MoveOnScreen(wxPoint position)
{
    VAR_DEBUG(position);
    int count = 0;
    while (wxGetMouseState().GetPosition() != position)
    {
        // Loop is required since sometimes the move fails the first time.
        // Particularly seen when working through remote desktop/using touchpad.
        wxUIActionSimulator().MouseMove(position);
        waitForIdle();
        if (++count > 3) break;
    }
    waitForIdle();
    if (wxGetMouseState().GetPosition() != position)
    {
        // When connecting via RDP (Windows remote desktop) the assert below sometimes fails,
        // even if the loop above was exited with a correct mouse position. Try to move to the
        // correct position at least once.
        wxUIActionSimulator().MouseMove(position);
        waitForIdle();
    }
    ASSERT_EQUALS(wxGetMouseState().GetPosition(), position);
}

void ClickTopLeft(wxWindow* window, wxPoint extraoffset)
{
    MoveOnScreen(window->GetScreenPosition() + extraoffset);
    wxUIActionSimulator().MouseClick();
    waitForIdle();
}

void ClickBottomLeft(wxWindow* window, wxPoint extraoffset)
{
    wxRect r = window->GetRect();
    wxPoint p = window->GetScreenPosition();
    p.y += r.height;
    MoveOnScreen(p + extraoffset);
    wxUIActionSimulator().MouseClick();
    waitForIdle();

}

} // namespace
