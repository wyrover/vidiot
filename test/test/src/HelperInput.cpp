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

//////////////////////////////////////////////////////////////////////////
// WAIT HANDLING
//////////////////////////////////////////////////////////////////////////

struct WaitHelper
{
    WaitHelper() 
        : Value(true)
    {
    }
    static WaitHelper& Get()
    {
        static WaitHelper sInstance;
        return sInstance;
    }
    static void ExecuteAndWait(boost::function<void()> f)
    {
        f();
        if (WaitHelper::Get().Value)
        {
            WaitForIdle();
        }
    }
    bool Value;
};

void SetWaitAfterEachInputAction(bool wait)
{
    ASSERT_DIFFERS(WaitHelper::Get().Value,wait);
    WaitHelper::Get().Value = wait;
}


//////////////////////////////////////////////////////////////////////////
// PHYSICAL EVENTS (UIACTIONSIMULATOR)
//////////////////////////////////////////////////////////////////////////

void MouseLeftDown()
{
    ASSERT(FixtureGui::UseRealUiEvents);
    WaitHelper::Get().ExecuteAndWait( [] { wxUIActionSimulator().MouseDown(); });
}

void MouseLeftUp()
{
    ASSERT(FixtureGui::UseRealUiEvents);
    WaitHelper::Get().ExecuteAndWait( [] { wxUIActionSimulator().MouseUp(); });
}

void MouseMoveWithinWidget(wxPoint position, wxPoint origin)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    VAR_DEBUG(position)(origin);
    wxPoint absoluteposition = origin + position;
    MouseMoveOnScreen(absoluteposition);
}

void MouseMoveOnScreen(wxPoint position)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    VAR_DEBUG(position);
    int count = 0;

    while (wxGetMouseState().GetPosition() != position)
    {
        // Loop is required since sometimes the move fails the first time.
        // Particularly seen when working through remote desktop/using touchpad.
        WaitHelper::Get().ExecuteAndWait( [position] { wxUIActionSimulator().MouseMove(position); });
        if (++count > 3) break;
    }
    ASSERT_EQUALS(wxGetMouseState().GetPosition(), position);
}

void MouseMoveRelative(wxPoint offset)
{
    MouseMoveOnScreen(wxGetMouseState().GetPosition() + offset);
}

void MouseClickTopLeft(wxWindow* window, wxPoint extraoffset)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    MouseMoveOnScreen(window->GetScreenPosition() + extraoffset);
    wxUIActionSimulator().MouseClick();
    WaitForIdle();
}

void MouseClickBottomLeft(wxWindow* window, wxPoint extraoffset)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    wxRect r = window->GetRect();
    wxPoint p = window->GetScreenPosition();
    p.y += r.height;
    MouseMoveOnScreen(p + extraoffset);
    wxUIActionSimulator().MouseClick();
    WaitForIdle();

}

void KeyboardKeyPress(int keycode, int modifiers)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    wxUIActionSimulator().Char(keycode,modifiers);
    WaitForIdle();
}

void KeyboardKeyPressN(int count, int keycode, int modifiers)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    for (int i = 0; i < count; ++i) { KeyboardKeyPress(keycode,modifiers); }
}

//////////////////////////////////////////////////////////////////////////
// TIMELINE EVENTS
//////////////////////////////////////////////////////////////////////////

// Todo step 2 : create simulated mousestate (including modifier keys etc.) and generate events.
//               or: redesign such that these 'other' fields of wxevents are never used! (cached within Keyboard class)

struct CurrentTimelineInputState
{
    CurrentTimelineInputState()
        : ControlDown(false)
        , ShiftDown(false)
        , Position(0,0)
    {
    }

    static CurrentTimelineInputState& Get()
    {
        static CurrentTimelineInputState sInstance;
        return sInstance;
    }

    bool ControlDown;
    bool ShiftDown;
    wxPoint Position;
};

void TimelineMove(wxPoint position)
{
    if (FixtureGui::UseRealUiEvents)
    {
        MouseMoveWithinWidget(position,TimelinePosition() - getTimeline().getScrolling().getOffset());
        ASSERT_EQUALS(getTimeline().getMouse().getVirtualPosition(), position);
    }
    else
    {
    }
}


void TimelineMoveRight(pixel length)
{
    if (FixtureGui::UseRealUiEvents)
    {
        ASSERT(FixtureGui::UseRealUiEvents);
        MouseMoveRelative(wxPoint(length,0));
    }
    else
    {
    }
}

void TimelineMoveLeft(pixel length)
{
    if (FixtureGui::UseRealUiEvents)
    {
        ASSERT(FixtureGui::UseRealUiEvents);
        MouseMoveRelative(wxPoint(-length,0));
    }
    else
    {
    }
}


void TimelineLeftClick(wxPoint position)
{
    TimelineMove(position);
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([] {wxUIActionSimulator().MouseClick(wxMOUSE_BTN_LEFT);} );
        ASSERT_EQUALS(getTimeline().getMouse().getLeftDownPosition(), position);
    }
    else
    {
    }
}

void TimelineLeftDown()
{
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([] {wxUIActionSimulator().MouseDown(wxMOUSE_BTN_LEFT);} );
    }
    else
    {
    }
}

void TimelineLeftUp()
{
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([] {wxUIActionSimulator().MouseUp(wxMOUSE_BTN_LEFT);} );
    }
    else
    {
    }
}

void TimelineRightClick(wxPoint position)
{
    TimelineMove(position);
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([] {wxUIActionSimulator().MouseClick(wxMOUSE_BTN_RIGHT);} );
        ASSERT_EQUALS(getTimeline().getMouse().getVirtualPosition(), position);
    }
    else
    {
    }
}



void TimelineRightDown()
{
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([] {wxUIActionSimulator().MouseDown(wxMOUSE_BTN_RIGHT);} );
    }
    else
    {
    }
}

void TimelineRightUp()
{
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([] {wxUIActionSimulator().MouseUp(wxMOUSE_BTN_RIGHT);} );
    }
    else
    {
    }
}

void TimelineKeyDown(int key)
{
    if (FixtureGui::UseRealUiEvents)
    {
        switch (key)
        {
        case wxMOD_ALT:
        case wxMOD_SHIFT:
        case wxMOD_CONTROL:
            WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().KeyDown(0,key);} );
            break;
        default:
            WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().KeyDown(key);} );
        }
    }
    else
    {
        switch (key)
        {
        case wxMOD_SHIFT:
            ASSERT(!CurrentTimelineInputState::Get().ShiftDown);
            CurrentTimelineInputState::Get().ShiftDown = true;
            break;
        case wxMOD_CONTROL:
            ASSERT(!CurrentTimelineInputState::Get().ControlDown);
            CurrentTimelineInputState::Get().ControlDown = true;
            break;
        }
        // generate event
    }
}

void TimelineKeyUp(int key)
{
    if (FixtureGui::UseRealUiEvents)
    {
        switch (key)
        {
        case wxMOD_ALT:
        case wxMOD_SHIFT:
        case wxMOD_CONTROL:
            WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().KeyUp(0,key);} );
            break;
        default:
            WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().KeyUp(key);} );
        }
        WaitForIdle();
    }
    else
    {
        switch (key)
        {
        case wxMOD_SHIFT:
            ASSERT(CurrentTimelineInputState::Get().ShiftDown);
            CurrentTimelineInputState::Get().ShiftDown = false;
            break;
        case wxMOD_CONTROL:
            ASSERT(CurrentTimelineInputState::Get().ControlDown);
            CurrentTimelineInputState::Get().ControlDown = false;
            break;
        }
        // generate event
    }
}

void TimelineKeyPress(int key)
{
    if (FixtureGui::UseRealUiEvents)
    {
        switch (key)
        {
        case wxMOD_SHIFT:
        case wxMOD_CONTROL:
            WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().Char(0,key);} );
            break;
        default:
            WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().Char(key);} );
        }
    }
    else
    {
        TimelineKeyDown(key);
        TimelineKeyUp(key);
    }
}

void TimelineKeyPressN(int count, int keycode)
{
    for (int i = 0; i < count; ++i) { TimelineKeyPress(keycode); }
}

} // namespace
