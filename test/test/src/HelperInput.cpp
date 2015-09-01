// Copyright 2014-2015 Eric Raijmakers.
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
    static void ExecuteAndWait(std::function<void()> f)
    {
        f();
        if (WaitHelper::Get().Value)
        {
            WaitForIdle;
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
    WaitForIdle;
}

void MouseClickBottomLeft(wxWindow* window, wxPoint extraoffset)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    wxRect r = window->GetRect();
    wxPoint p = window->GetScreenPosition();
    p.y += r.height;
    MouseMoveOnScreen(p + extraoffset);
    wxUIActionSimulator().MouseClick();
    WaitForIdle;

}

void KeyboardKeyPress(int keycode, int modifiers)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    wxUIActionSimulator().Char(keycode,modifiers);
    WaitForIdle;
}

void KeyboardKeyPressN(int count, int keycode, int modifiers)
{
    ASSERT(FixtureGui::UseRealUiEvents);
    for (int i = 0; i < count; ++i) { KeyboardKeyPress(keycode,modifiers); }
}

//////////////////////////////////////////////////////////////////////////
// TIMELINE EVENTS
//////////////////////////////////////////////////////////////////////////

struct CurrentTimelineInputState
{
    CurrentTimelineInputState()
        : ControlDown(false)
        , ShiftDown(false)
        , AltDown(false)
        , LeftDown(false)
        , RightDown(false)
        , Position(0,0)
    {
    }

    static CurrentTimelineInputState& Get()
    {
        static CurrentTimelineInputState sInstance;
        return sInstance;
    }

    wxMouseState getWxMouseState() const
    {
        wxMouseState state;
        state.SetControlDown(ControlDown);
        state.SetShiftDown(ShiftDown);
        state.SetAltDown(AltDown);
        state.SetLeftDown(LeftDown);
        state.SetRightDown(RightDown);
        state.SetPosition(Position);
        return state;
    }

    bool ControlDown;
    bool ShiftDown;
    bool AltDown;
    bool LeftDown;
    bool RightDown;
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
        util::thread::RunInMainAndWait([&position]
        {
            CurrentTimelineInputState::Get().Position = position  - getTimeline().getScrolling().getOffset();
            gui::timeline::state::EvMotion event(CurrentTimelineInputState::Get().getWxMouseState());
            getTimeline().getStateMachine().handleMotion(event);
        });
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
        util::thread::RunInMainAndWait([&length]
        {
            CurrentTimelineInputState::Get().Position.x += length;
            gui::timeline::state::EvMotion event(CurrentTimelineInputState::Get().getWxMouseState());
            getTimeline().getStateMachine().handleMotion(event);
        });
    }
}

void TimelineMoveLeft(pixel length)
{
    TimelineMoveRight(-length);
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
        TimelineLeftDown();
        TimelineLeftUp();
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
        util::thread::RunInMainAndWait([]
        {
            ASSERT(!CurrentTimelineInputState::Get().LeftDown);
            CurrentTimelineInputState::Get().LeftDown = true;
            gui::timeline::state::EvLeftDown event(CurrentTimelineInputState::Get().getWxMouseState());
            getTimeline().getStateMachine().handleLeftDown(event);
        });
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
        util::thread::RunInMainAndWait([]
        {
            ASSERT(CurrentTimelineInputState::Get().LeftDown);
            CurrentTimelineInputState::Get().LeftDown = false;
            gui::timeline::state::EvLeftUp event(CurrentTimelineInputState::Get().getWxMouseState());
            getTimeline().getStateMachine().handleLeftUp(event);
        });
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
        TimelineRightDown();
        TimelineRightUp();
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
        util::thread::RunInMainAndWait([]
        {
            ASSERT(!CurrentTimelineInputState::Get().RightDown);
            CurrentTimelineInputState::Get().RightDown = true;
            gui::timeline::state::EvRightDown event(CurrentTimelineInputState::Get().getWxMouseState());
            getTimeline().getStateMachine().handleRightDown(event);
        });
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
        util::thread::RunInMainAndWait([]
        {
            ASSERT(CurrentTimelineInputState::Get().RightDown);
            CurrentTimelineInputState::Get().RightDown = false;
            gui::timeline::state::EvRightUp event(CurrentTimelineInputState::Get().getWxMouseState());
            getTimeline().getStateMachine().handleRightUp(event);
        });
    }
}

wxKeyModifier KeycodeToModifier(int key)
{
    switch (key)
    {
    case WXK_ALT: return wxMOD_ALT;
    case WXK_SHIFT: return wxMOD_SHIFT;
    case WXK_CONTROL: return wxMOD_CONTROL;
    }
    return wxMOD_NONE;
}

void TimelineKeyDown(int key)
{
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().KeyDown(key,KeycodeToModifier(key));} );
    }
    else
    {
        util::thread::RunInMainAndWait([key]
        {
            switch (key)
            {
            case WXK_SHIFT:
                ASSERT(!CurrentTimelineInputState::Get().ShiftDown);
                CurrentTimelineInputState::Get().ShiftDown = true;
                break;
            case WXK_CONTROL:
                ASSERT(!CurrentTimelineInputState::Get().ControlDown);
                CurrentTimelineInputState::Get().ControlDown = true;
                break;
            case WXK_ALT:
                ASSERT(!CurrentTimelineInputState::Get().AltDown);
                CurrentTimelineInputState::Get().AltDown = true;
                break;
            }
            wxMouseState state = CurrentTimelineInputState::Get().getWxMouseState();
            gui::timeline::state::EvKeyDown event(state,key);
            getTimeline().getStateMachine().handleKeyDown(event);
        });
    }
}

void TimelineKeyUp(int key)
{
    if (FixtureGui::UseRealUiEvents)
    {
        WaitHelper::ExecuteAndWait([key] {wxUIActionSimulator().KeyUp(key,KeycodeToModifier(key));} );
    }
    else
    {
        util::thread::RunInMainAndWait([key]
        {
            switch (key)
            {
            case WXK_SHIFT:
                ASSERT(CurrentTimelineInputState::Get().ShiftDown);
                CurrentTimelineInputState::Get().ShiftDown = false;
                break;
            case WXK_CONTROL:
                ASSERT(CurrentTimelineInputState::Get().ControlDown);
                CurrentTimelineInputState::Get().ControlDown = false;
                break;
            case WXK_ALT:
                ASSERT(CurrentTimelineInputState::Get().AltDown);
                CurrentTimelineInputState::Get().AltDown = false;
                break;
            }
            wxMouseState state = CurrentTimelineInputState::Get().getWxMouseState();
            gui::timeline::state::EvKeyUp event(state,key);
            getTimeline().getStateMachine().handleKeyUp(event);
        });
    }
}

void TimelineKeyPress(int key)
{
    TimelineKeyDown(key);
    TimelineKeyUp(key);
}

void TimelineKeyPressN(int count, int keycode)
{
    for (int i = 0; i < count; ++i) { TimelineKeyPress(keycode); }
}

void TimelineRightMouseScroll(pixel length)
{
    ASSERT(!FixtureGui::UseRealUiEvents);
    ASSERT_LESS_THAN(std::abs(length), getTimeline().GetSize().GetWidth() / 2);
    TimelineMove(wxPoint(getTimeline().getScrolling().getOffset().x + getTimeline().GetSize().GetWidth() / 2, CurrentTimelineInputState::Get().Position.y));
    TimelineRightDown();
    TimelineMoveRight(length);
    TimelineRightUp();
}

} // namespace
