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

#include "Test.h"

namespace test {

void GiveKeyboardFocus(wxWindow* widget)
{
#ifdef _MSC_VER
    if (dynamic_cast<wxSpinCtrlDouble*>(widget) != 0)
    {
        MouseClickTopLeft(widget,wxPoint(2,2));
    }
    else if (dynamic_cast<wxSpinCtrl*>(widget) != 0)
    {
        MouseClickTopLeft(widget,wxPoint(2,2));
    }
    else if (dynamic_cast<wxSlider*>(widget) != 0)
    {
        MouseClickTopLeft(widget);
    }
    else
    {
        NIY("Unknown");
    }
#else
    RunInMainAndWait([widget] { widget->SetFocus(); });
    WaitForIdle();
    if (dynamic_cast<wxSpinCtrlDouble*>(widget) != 0)
    {
        MouseClickTopLeft(widget, wxPoint(8,8));
    }
    WaitForIdle();
#endif
}

void SetValue(wxSlider* widget, int value)
{
    widget->SetValue(value);
    widget->GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_SLIDER));
    WaitForIdle();
}

void SetValue(wxSpinCtrl* widget, int value)
{
    widget->SetValue(value);
    wxSpinEvent* event = new wxSpinEvent(wxEVT_SPINCTRL,0);
    event->SetValue(value);
    widget->GetEventHandler()->QueueEvent(event);
    WaitForIdle();
}

void SetValue(wxSpinCtrlDouble* widget, double value)
{
    widget->SetValue(value);
    widget->GetEventHandler()->QueueEvent(new wxSpinDoubleEvent(wxEVT_SPINCTRLDOUBLE,0,value));
    WaitForIdle();
}

} // namespace
