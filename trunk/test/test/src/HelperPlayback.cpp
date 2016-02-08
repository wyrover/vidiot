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

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WaitForPlayback::WaitForPlayback(bool waitForStart)
    :   mWaitForStart(waitForStart)
    ,   mDone(false)
{
    util::thread::RunInMainAndWait([this]
    {
        getTimeline().getPlayer()->Bind(gui::EVENT_PLAYBACK_ACTIVE, &WaitForPlayback::onPlaybackActive, this);
    });
}
WaitForPlayback::~WaitForPlayback()
{
    util::thread::RunInMainAndWait([this]
    {
        getTimeline().getPlayer()->Unbind(gui::EVENT_PLAYBACK_ACTIVE, &WaitForPlayback::onPlaybackActive, this);
    });
}

//////////////////////////////////////////////////////////////////////////
// WAIT
//////////////////////////////////////////////////////////////////////////

void WaitForPlayback::wait()
{
    boost::mutex::scoped_lock lock(mMutex);
    while (!mDone)
    {
        mCondition.wait(lock);
    }
}
//////////////////////////////////////////////////////////////////////////
// EVENT FROM PLAYER
//////////////////////////////////////////////////////////////////////////

void WaitForPlayback::onPlaybackActive(gui::PlaybackActiveEvent& event)
{
    if (event.getValue() == mWaitForStart)
    {
        mDone = true;
        boost::mutex::scoped_lock lock(mMutex);
        mCondition.notify_all();
    }
    event.Skip();
}

void Play(pixel from, int ms)
{
    TimelinePositionCursor(from);
    Play(ms);
}

void Play(int ms)
{
    WaitForPlayback started(true);
    WaitForPlayback stopped(false);
    // NOTE: Don't use WaitForIdle() when the video is playing!!!
    //       When the video is playing, the system does not become Idle (playback events).
    // NOTE: Starting and stopping the playback is not done via space key presses.
    //       Space does a 'toggle', which sometimes causes irratic behavior.

    util::thread::RunInMainScheduler::get().run([] { getTimeline().getPlayer()->play(); });
    started.wait();
    pause(ms);
    util::thread::RunInMainScheduler::get().run([] { getTimeline().getPlayer()->stop(); });
    stopped.wait();
}

void MaximizePreviewPane(bool maximizeWindow, bool hideDetails)
{
    WindowTriggerMenu(ID_SHOW_PROJECT);
    if (hideDetails) 
    { 
        WindowTriggerMenu(ID_SHOW_DETAILS); 
    }
    wxRect r{ getTimeline().getPlayer()->GetScreenRect() };
    wxPoint p{ r.GetLeft() + r.GetWidth() / 2, r.GetBottom() + 4 };

    util::thread::RunInMainAndWait([maximizeWindow]
    {
        if (maximizeWindow)
        {
            gui::Window::get().Maximize();
        }

        // Make preview as large as possible. Trick taken from http://trac.wxwidgets.org/ticket/13180
        // wxAUI hack: set minimum height to desired value, then call wxAuiPaneInfo::Fixed() to apply it
        gui::Window::get().getUiManager().GetPane(gui::Window::sPaneNamePreview).MinSize(-1, 510);
        gui::Window::get().getUiManager().GetPane(gui::Window::sPaneNamePreview).Fixed();
        gui::Window::get().getUiManager().Update();
        //now make resizable again
        gui::Window::get().getUiManager().GetPane(gui::Window::sPaneNameTimelines).Resizable();
        gui::Window::get().getUiManager().Update();
    });
    WaitForIdle;
}

} // namespace
