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

#pragma once

#include "Test.h"

namespace test {

/// Trigger the menu given. That menu should be in the main menu bar.
/// \param id menu identifier
void WindowTriggerMenu(int id);

/// Trigger the menu given on the window given.
/// \param id menu identifier
void WindowTriggerMenu(wxWindow& window, int id);

/// Check the menu given. That menu should be in the main menu bar.
/// Note that the corresponding event will always be triggered, even
/// if the control indicated with 'id' already has the value indicated
/// by 'checked'.
/// \param id menu identifier
/// \param checked if true, the menu is checked, unchecked otherwise
void WindowCheckMenu(int id, bool checked = true);

/// Check the menu given on the window given.
/// Note that the corresponding event will always be triggered, even
/// if the control indicated with 'id' already has the value indicated
/// by 'checked'.
/// \param id menu identifier
/// \param checked if true, the menu is checked, unchecked otherwise
void WindowCheckMenu(wxFrame& window, int id, bool checked = true);

struct OpenProjectWaiter
: public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
    OpenProjectWaiter();
    ~OpenProjectWaiter();
    void onOpenProject(model::EventOpenProject &event );
    void wait();
private:

    boost::condition_variable mCondition;
    boost::mutex mMutex;
    std::atomic<bool> mDone;
};

/// Create a new project in a blank application by triggering File->New
/// \return root node of the project
model::FolderPtr WindowCreateProject();

/// Return the currently active sequence menu
wxMenu* getSequenceMenu();

/// Trigger an undo via the menu
/// \param steps number of times an Undo is triggered
void Undo(int steps = 1);

/// Trigger a redo via the menu
/// \param steps number of times an Undo is triggered
void Redo(int steps = 1);

/// Use this to set a breakpoint, after which the window is given focus
/// again so that the test case won't fail. This can be used to enable
/// specific other breakpoints at desired times (particularly, after
/// certain init code has been done.
void BREAK();

void LogHistory();

void WaitUntilMainWindowActive(bool active);
void WaitUntilDialogOpen(bool open);

} // namespace
