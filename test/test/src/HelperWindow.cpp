// Copyright 2013-2016 Eric Raijmakers.
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
    VAR_DEBUG(id);
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
    WaitForIdle;
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
    util::thread::RunInMainAndWait([&window, id, checked]
    {
        window.GetMenuBar()->Check(id, !checked);
        window.ProcessCommand(id);
    });
}

OpenProjectWaiter::OpenProjectWaiter()
    : mDone(false)
{
    util::thread::RunInMainAndWait([this]
    {
        gui::Window::get().Bind(model::EVENT_OPEN_PROJECT, &OpenProjectWaiter::onOpenProject, this);
    });
}

OpenProjectWaiter::~OpenProjectWaiter()
{
    util::thread::RunInMainAndWait([this]
    {
        gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT, &OpenProjectWaiter::onOpenProject, this);
    });
}

void OpenProjectWaiter::onOpenProject(model::EventOpenProject &event)
{
    event.Skip();
    mDone = true;
    mCondition.notify_all();
}

void OpenProjectWaiter::wait()
{
    boost::mutex::scoped_lock lock(mMutex);
    while (!mDone)
    {
        mCondition.wait(lock);
    }
}

model::FolderPtr WindowCreateProject()
{
    WaitForIdle;
    OpenProjectWaiter waitForOpenedProject;
    WindowTriggerMenu(wxID_NEW);
    waitForOpenedProject.wait();
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
    wxList& cmds = model::CommandProcessor::get().GetCommands();
    wxList::compatibility_iterator it = cmds.GetFirst();
    while (it != cmds.GetLast())
    {
        wxObject* command = it->GetData();
        std::string type = typeid(*command).name();
        VAR_DEBUG(command)(type);
        it = it->GetNext();
    }
    wxCommand* current = model::CommandProcessor::get().GetCurrentCommand();
    VAR_DEBUG(current);
}

void WaitUntilMainWindowActive(bool active)
{
    while ( active != util::thread::RunInMainReturning<bool>([] { return wxTheApp->GetMainLoop()->IsRunning(); }) )
    {
        pause(50);
    }
}

void WaitUntilDialogOpen(bool open)
{
    while ( open != gui::Window::get().isDialogOpen() )
    {
        pause(50);
    }
    WaitForIdle;
}

} // namespace
