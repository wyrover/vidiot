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
    MaximizePreviewPane();

    StartTest("Playback");
    TimelinePositionCursor(RightPixel(VideoClip(1,1)) - 10);
    for (int i = 0; i < 8; ++i)
    {
        Play(500);
    }

    Undo(3);
}

void TestPlayback::testPlaybackWithDifferentSpeed()
{
    StartTestSuite();
    util::thread::RunInMainAndWait([] { getTimeline().getPlayer()->setSpeed(50); });
    TimelinePositionCursor(HCenter(VideoClip(0, 3)));
    Play(1000);
    util::thread::RunInMainAndWait([] { getTimeline().getPlayer()->setSpeed(110); });
    TimelinePositionCursor(HCenter(VideoClip(0, 3)));
    Play(1000);
    util::thread::RunInMainAndWait([] { getTimeline().getPlayer()->setSpeed(150); });
    TimelinePositionCursor(HCenter(VideoClip(0, 3)));
    Play(1000);
}

void TestPlayback::testPlaybackAfterRangedPlayback()
{
    StartTestSuite();
    MakeInOutTransitionAfterClip preparation(3);
    TimelineLeftClick(Center(VideoClip(0, 4))); // Open properties
    WaitForPlaybackStarted started;
    ButtonTriggerPressed(DetailsClipView()->getPlayButton());
    started.wait();
    pause(1000);
    WaitForPlaybackStopped stopped;
    TimelinePositionCursor(HCenter(VideoClip(0, 6)));
    Play(1000);
    stopped.wait();
}

} // namespace
