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

#ifndef HELPER_WINDOW_H
#define HELPER_WINDOW_H

#include "Test.h"

namespace test {

/// Trigger the menu given. That menu should be in the main menu bar.
/// \param id menu identifier
void triggerMenu(int id);

/// Trigger the menu given on the window given.
/// \param id menu identifier
void triggerMenu(wxWindow& window, int id);

/// Check the menu given. That menu should be in the main menu bar.
/// Note that the corresponding event will always be triggered, even
/// if the control indicated with 'id' already has the value indicated
/// by 'checked'.
/// \param id menu identifier
/// \param checked if true, the menu is checked, unchecked otherwise
void checkMenu(int id, bool checked = true);

/// Check the menu given on the window given.
/// Note that the corresponding event will always be triggered, even
/// if the control indicated with 'id' already has the value indicated
/// by 'checked'.
/// \param id menu identifier
/// \param checked if true, the menu is checked, unchecked otherwise
void checkMenu(wxFrame& window, int id, bool checked = true);

/// Create a new project in a blank application by triggering File->New
/// \return root node of the project
model::FolderPtr createProject();

/// Return the currently active sequence menu
wxMenu* getSequenceMenu();

/// Trigger an undo via pressing CTRL-z
/// \param steps number of times an Undo is triggered
void Undo(int steps = 1);

/// Trigger a redo via pressing CTRL-y
/// \param steps number of times an Undo is triggered
void Redo(int steps = 1);

/// Return current command in the history
wxCommand* getCurrentCommand();

/// Assert if the current active command in the history
/// (the command that will be undone if undo is triggered)
/// is of the given type.
template <class COMMAND>
void ASSERT_CURRENT_COMMAND_TYPE()
{
    wxCommand* cmd = 0;
    RunInMainAndWait([&]
    {
        cmd = getCurrentCommand(); // Split to make debugging easier (inspect cmd to see what the current command is in case of failure)
    });
    ASSERT(cmd);
    COMMAND* command = dynamic_cast<COMMAND*>(cmd);
    if (!command)
    {
        const char* Expected = (typeid(COMMAND).name());
        const char* Actual = (typeid(*cmd).name());
        ASSERT(command)(Expected)(Actual);
    }
};

/// Use this to set a breakpoint, after which the window is given focus
/// again so that the test case won't fail. This can be used to enable
/// specific other breakpoints at desired times (particularly, after
/// certain init code has been done.
void BREAK();

/// Log the command history
void logHistory();

/// Press down left mouse button
/// \param wait if true then a wait for idle is done
void LeftDown(bool wait = true);

/// Release left mouse button
/// \param wait if true then a wait for idle is done
void LeftUp(bool wait = true);

/// Press down left mouse button
/// \param wait if true then a wait for idle is done
void RightDown(bool wait = true);

/// Release left mouse button
/// \param wait if true then a wait for idle is done
void RightUp(bool wait = true);

/// Press and release left mouse button
/// \param wait if true then a wait for idle is done
void ClickLeft(bool wait = true);

/// Press and release right mouse button
/// \param wait if true then a wait for idle is done
void ClickRight(bool wait = true);

void ControlDown();     ///< Press down CTRL
void ControlUp();       ///< Release CTRL

void ShiftDown();       ///< Press down Shift
void ShiftUp();         ///< Release Shift

void Type(int keycode, int modifiers = wxMOD_NONE);
void TypeN(int count, int keycode, int modifiers = wxMOD_NONE);

/// Move the mouse to the right, the given number of pixels
/// \param length number of pixels the mouse is moved to the right
void MoveRight(pixel length);

/// Move the mouse to the left, the given number of pixels
/// \param length number of pixels the mouse is moved to the left
void MoveLeft(pixel length);

/// Move the mouse to the given position
/// \param origin Use the origin to specify within which widget the position is offset (default: within the timeline)
/// \param position Origin of the widget above which the mouse must be moved
void MoveWithinWidget(wxPoint position, wxPoint origin);

/// Move the mouse to an absolute position on screen
/// \param position position wrt top-left corner of screen
void MoveOnScreen(wxPoint position);

/// Click on the top left pixel of a widget to give it the focus.
/// \param window window that must receive the focus after clicking
/// \param extraoffset extra offset (some widgets require this)
void ClickTopLeft(wxWindow* window, wxPoint extraoffset = wxPoint(0,0));

/// Click on the bottom left pixel of a widget to give it the focus.
/// \param window window that must receive the focus after clicking
/// \param extraoffset extra offset (some widgets require this)
void ClickBottomLeft(wxWindow* window, wxPoint extraoffset = wxPoint(0,0));

} // namespace

#endif
