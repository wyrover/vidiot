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
    PositionCursor(RightPixel(VideoTrack(0)) - 5);
    WaitForPlayback playbackstarted(true);
    WaitForPlayback playbackstopped(false);
    Type(' ');
    playbackstarted.wait();
    playbackstopped.wait();
}

void TestPlayback::testPlaybackComplexSequence()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);

    StartTest("Preparation: Add transition to test skipping frames for a transition.");
    MakeInOutTransitionAfterClip preparation(3);

    StartTest("Preparation: Make a video clip in another track to test that skipping compositions works.");
    DragToTrack(1,VideoClip(0,6),model::IClipPtr()); //
    Drag(From(Center(VideoClip(1,1))).AlignLeft(LeftPixel(VideoClip(0,2))));
    Click(Center(VideoClip(1,1)));
    ClickTopLeft(DetailsClipView()->getOpacitySlider()); // Give focus
    TypeN(3,WXK_PAGEUP);

    StartTest("Preparation: Enlarge preview as much as possible to make the decoded video size as large as possible.");
    triggerMenu(ID_SHOW_PROJECT);
    triggerMenu(ID_SHOW_DETAILS);
    wxRect r = getTimeline().getPlayer()->GetScreenRect();
    wxPoint p(r.GetLeft() + r.GetWidth() / 2, r.GetBottom() + 4);
    MoveOnScreen(p);
    LeftDown();
    MoveOnScreen(p + wxPoint(0,200));
    LeftUp();

    StartTest("Playback");
    PositionCursor(RightPixel(VideoClip(1,1)) - 10);
    for (int i = 0; i < 8; ++i)
    {
        WaitForPlayback playbackstarted(true);
        WaitForPlayback playbackstopped(false);
        Type(' ');
        playbackstarted.wait();
        pause(500);
        Type(' ');
        playbackstopped.wait();
    }

    Undo(3);
}

} // namespaceh