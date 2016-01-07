// Copyright 2014-2016 Eric Raijmakers.
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

//////////////////////////////////////////////////////////////////////////
// WAIT HANDLING
//////////////////////////////////////////////////////////////////////////

/// Enable (true) or disable (false) the WaitForIdle after each (simulated)
/// user action, in one of the TimelineLeft*, TimelineRight*, and TimelineKey* 
/// methods below.
void SetWaitAfterEachInputAction(bool wait = true);

//////////////////////////////////////////////////////////////////////////
// PHYSICAL EVENTS (UIACTIONSIMULATOR)
//////////////////////////////////////////////////////////////////////////

/// Press down left mouse button
void MouseLeftDown();

/// Release left mouse button
void MouseLeftUp();

/// Move the mouse to the given position
/// \param origin Use the origin to specify within which widget the position is offset (default: within the timeline)
/// \param position Origin of the widget above which the mouse must be moved
void MouseMoveWithinWidget(wxPoint position, wxPoint origin);

/// Move the mouse to an absolute position on screen
/// \param position position wrt top-left corner of screen
void MouseMoveOnScreen(wxPoint position);

/// Click on the top left pixel of a widget to give it the focus.
/// \param window window that must receive the focus after clicking
/// \param extraoffset extra offset (some widgets require this)
void MouseClickTopLeft(wxWindow* window, wxPoint extraoffset = wxPoint(0,0));

/// Click on the bottom left pixel of a widget to give it the focus.
/// \param window window that must receive the focus after clicking
/// \param extraoffset extra offset (some widgets require this)
void MouseClickBottomLeft(wxWindow* window, wxPoint extraoffset = wxPoint(0,0));

void KeyboardKeyPress(int keycode, int modifiers = wxMOD_NONE);
void KeyboardKeyPressN(int count, int keycode, int modifiers = wxMOD_NONE);

//////////////////////////////////////////////////////////////////////////
// TIMELINE EVENTS
//////////////////////////////////////////////////////////////////////////

/// Trigger a move to the given position.
/// \param position mouse is moved to this position the events occur
void TimelineMove(wxPoint position);

/// Trigger a move in the timeline (relative to the current position in horizontal direction).
/// \param length number of pixels (relative to current mouse position) the mouse is moved to the left.
void TimelineMoveLeft(pixel length);

/// Trigger a move in the timeline (relative to the current position in horizontal direction).
/// \param length number of pixels (relative to current mouse position) the mouse is moved to the right.
void TimelineMoveRight(pixel length);

/// Trigger a move to the given position followed by left down and up events.
/// \param position at this position the events occur
void TimelineLeftClick(wxPoint position);

/// Trigger a left down event in the timeline at the current mouse position.
void TimelineLeftDown();

/// Trigger a left up event in the timeline at the current mouse position.
void TimelineLeftUp();

/// Trigger a move to the given position followed by right down and up events.
/// \param position at this position the events occur
void TimelineRightClick(wxPoint position);

/// Trigger a right down event in the timeline at the current mouse position.
void TimelineRightDown();

/// Trigger a right up event in the timeline at the current mouse position.
void TimelineRightUp();

/// Trigger a key down event in the timeline for the given key.
/// \param key wx key code for key (may be wxMOD_* to generate events for modifiers)
void TimelineKeyDown(int key); 

/// Trigger a key up event in the timeline for the given key.
/// \param key wx key code for key (may be wxMOD_* to generate events for modifiers)
void TimelineKeyUp(int key);   

/// Trigger a key down then up event in the timeline for the given key.
/// \param key wx key code for key (may be wxMOD_* to generate events for modifiers)
void TimelineKeyPress(int key);

/// Trigger a key down then up event in the timeline for the given key.
/// \param key wx key code for key (may be wxMOD_* to generate events for modifiers)
/// \param count number of times the event is generated
void TimelineKeyPressN(int count, int key);

void TimelineRightMouseScroll(pixel length);

} // namespace
