#include "HelperWindow.h"

#include <wx/uiaction.h>
#include "HelperApplication.h"
#include "Project.h"
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

void Undo()
{
    LOG_DEBUG;
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    wxUIActionSimulator().Char('z');
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    waitForIdle();
    logHistory();
}

void Redo()
{
    LOG_DEBUG;
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    wxUIActionSimulator().Char('y');
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    waitForIdle();
    logHistory();
}

wxCommand* getCurrentCommand()
{
    return gui::Window::get().GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->GetCurrentCommand();
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
    wxUIActionSimulator().Char(keycode);
    waitForIdle();
}
} // namespace