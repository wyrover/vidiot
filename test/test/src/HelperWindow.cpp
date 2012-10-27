#include "HelperWindow.h"

#include <wx/uiaction.h>
#include "HelperApplication.h"
#include "Project.h"

#include "UtilLogWxwidgets.h"
#include "Window.h"

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

model::FolderPtr getRoot()
{
    model::FolderPtr root = model::Project::get().getRoot();
    return boost::static_pointer_cast<model::Folder>(root);
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
    __asm { int 3 };
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

void LeftDown()
{
    wxUIActionSimulator().MouseDown();
    waitForIdle();
}

void LeftUp()
{
    wxUIActionSimulator().MouseUp();
    waitForIdle();
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