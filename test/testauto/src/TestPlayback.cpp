// Copyright 2013-2015 Eric Raijmakers.
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

#include "TestPlayback.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestPlayback::setUp()
{
    mProjectFixture.init();
}

void TestPlayback::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestPlayback::testPlaybackUntilEndOfSequence()
{
    StartTestSuite();
    TimelinePositionCursor(RightPixel(VideoTrack(0)) - 5);
    WaitForPlayback playbackstarted(true);
    WaitForPlayback playbackstopped(false);
    TimelineKeyPress(' ');
    playbackstarted.wait();
    playbackstopped.wait();
}

void TestPlayback::testPlaybackComplexSequence()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDVIDEOTRACK);

    StartTest("Preparation: Add transition to test skipping frames for a transition.");
    MakeInOutTransitionAfterClip preparation(3);

    StartTest("Preparation: Make a video clip in another track to test that skipping compositions works.");
    TimelineDragToTrack(1,VideoClip(0,6),model::IClipPtr()); //
    TimelineDrag(From(Center(VideoClip(1,1))).AlignLeft(LeftPixel(VideoClip(0,2))));
    TimelineLeftClick(Center(VideoClip(1,1)));
    SetValue(DetailsClipView()->getOpacitySlider(), 225); // Same as pressing 3 * PageUp

    StartTest("Preparation: Enlarge preview as much as possible to make the decoded video size as large as possible.");
    WindowTriggerMenu(ID_SHOW_PROJECT);
    WindowTriggerMenu(ID_SHOW_DETAILS);
    wxRect r = getTimeline().getPlayer()->GetScreenRect();
    wxPoint p(r.GetLeft() + r.GetWidth() / 2, r.GetBottom() + 4);

    util::thread::RunInMainAndWait([]
    {
        // Make preview as large as possible. Trick taken from http://trac.wxwidgets.org/ticket/13180
        // wxAUI hack: set minimum height to desired value, then call wxAuiPaneInfo::Fixed() to apply it
        gui::Window::get().getUiManager().GetPane(gui::Window::sPaneNamePreview).MinSize(-1,510);
        gui::Window::get().getUiManager().GetPane(gui::Window::sPaneNamePreview).Fixed();
        gui::Window::get().getUiManager().Update();
        //now make resizable again
        gui::Window::get().getUiManager().GetPane(gui::Window::sPaneNameTimelines).Resizable();
        gui::Window::get().getUiManager().Update();
    });
    WaitForIdle;

    StartTest("Playback");
    TimelinePositionCursor(RightPixel(VideoClip(1,1)) - 10);
    for (int i = 0; i < 8; ++i)
    {
        Play(500);
    }

    Undo(3);
}


// todo test playback with > speed and with < speed

} // namespace
