// Copyright 2014 Eric Raijmakers.
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

#ifndef HELPER_WIDGETS_H
#define HELPER_WIDGETS_H

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
    widget->SetSelection(widget->getIndex(value));
    widget->GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_CHOICE));
    waitForIdle();
}

} // namespace

#endif