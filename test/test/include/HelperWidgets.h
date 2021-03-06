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

/// Give focus to this widget (in order to receive keyboard events)
/// \param widget widget to receive focus
void GiveKeyboardFocus(wxWindow* widget);

/// Set the value of the control to the given value AND generate
/// proper event signaling the update to the application.
/// \param widget widget to be changed
/// \param value new value of the slider
void SetValue(wxSlider* widget, int value);

/// Set the value of the control to the given value AND generate
/// proper event signaling the update to the application.
/// \param widget widget to be changed
/// \param value new value of the slider
void SetValue(wxSpinCtrl* widget, int value);

/// Set the value of the control to the given value AND generate
/// proper event signaling the update to the application.
/// \param widget widget to be changed
/// \param value new value of the slider
void SetValue(wxSpinCtrlDouble* widget, double value);

/// Set the value of the control to the given value AND generate
/// proper event signaling the update to the application.
/// \param widget widget to be changed
/// \param value new value of the slider
template <class ITEMTYPE>
void SetValue(EnumSelector<ITEMTYPE>* widget, ITEMTYPE value)
{
    util::thread::RunInMainAndWait([widget, value]
    {
        widget->SetSelection(widget->getIndex(value));
        wxCommandEvent event(wxEVT_CHOICE);
        widget->GetEventHandler()->ProcessEvent(event);
    });
}

/// Simulate the setting of a check box to the given value.
/// \param widget box to be changed
/// \param state new state
void SetValue(wxCheckBox* widget, bool value);

/// Simulate the setting of a choice box to the given value.
/// \param widget box choice be changed
/// \param state new value
void SetValue(wxChoice* widget, wxString value);

/// Simulate the pressing of the given button.
/// \param button button for which command event 'pressed' is generated
void ButtonTriggerPressed(wxButton* button);

/// Simulate the pressing of the given button.
/// \param button button for which command event 'pressed' is generated
/// \param state state of the button when the event is received
void ButtonTriggerPressed(wxToggleButton* button, bool state = true);

} // namespace
