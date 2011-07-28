#include "HelperWindow.h"

#include <wx/uiaction.h>
#include "Application.h"
#include "AutoFolder.h"
#include "File.h"
#include "FixtureGui.h"
#include "ids.h"
#include "Project.h"
#include "ProjectView.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "UtilDialog.h"
#include "UtilLog.h"
#include "ViewMap.h"
#include "Window.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void HelperWindow::triggerMenu(int id)
{
    triggerMenu(gui::Window::get(), id);
}

void HelperWindow::triggerMenu(wxWindow& window, int id)
{
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
    waitForIdle();
}

void HelperWindow::waitForIdle()
{
    static_cast<gui::Application*>(wxTheApp)->waitForIdle();
}

model::FolderPtr HelperWindow::createProject()
{
    waitForIdle();
    triggerMenu(wxID_NEW);
    waitForIdle();
    return getRoot();
}

model::FolderPtr HelperWindow::getRoot()
{
    model::FolderPtr root = model::Project::get().getRoot();
    return boost::static_pointer_cast<model::Folder>(root);
}

wxString HelperWindow::randomString(int length)
{
    srand((unsigned)time(0)); 
    static const wxString alphanum = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";

    wxString result;
    for (int i = 0; i < length; ++i) 
    {
        result += alphanum.GetChar(rand() % (sizeof(alphanum) - 1));
    }
    return result;
}

void HelperWindow::pause(int ms)
{
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
}

model::SequencePtr HelperWindow::getSequence()
{
    return getTimeline().getSequence();
}

wxMenu* HelperWindow::getSequenceMenu()
{
    return gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex);
}

gui::timeline::Timeline& HelperWindow::getTimeline(model::SequencePtr sequence)
{
    return gui::TimelinesView::get().getTimeline(sequence);
}

void HelperWindow::triggerUndo()
{
    LOG_DEBUG;
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    wxUIActionSimulator().Char('z');
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    waitForIdle();
    logHistory();
}

void HelperWindow::triggerRedo()
{
    LOG_DEBUG;
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    wxUIActionSimulator().Char('y');
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    waitForIdle();
    logHistory();
}

void HelperWindow::logHistory()
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
        //pause(1000);
};

} // namespace