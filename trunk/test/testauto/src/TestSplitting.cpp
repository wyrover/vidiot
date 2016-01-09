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

#include "TestSplitting.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestSplitting::setUp()
{
    mProjectFixture.init();
}

void TestSplitting::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestSplitting::testSplitting()
{
    StartTestSuite();
    {
        TimelinePositionCursor(HCenter(VideoClip(0, 2)));
        TimelineKeyPress('s');
        ASSERT_SELECTION({ VideoClip(0,2), AudioClip(0,2) });
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 3));
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 3));
        Undo();
    }
    MakeInOutTransitionAfterClip preparation(1);
    {
        TimelinePositionCursor(HCenter(VideoClip(0,2)));
        TimelineKeyPress('s');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Undo();
    }
    {
        TimelinePositionCursor(LeftPixel(VideoClip(0,2)));
        TimelineKeyPress('s');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Undo();
    }
    {
        TimelinePositionCursor(RightPixel(VideoClip(0,2)));
        TimelineKeyPress('s');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Undo();
    }
}

void TestSplitting::testSplittingDuringPlayback()
{
    StartTestSuite();
    StartTest("Start playback");
    TimelinePositionCursor(HCenter(VideoClip(0,4)));
    WaitForPlaybackStarted started;
    TimelineKeyPress(' ');
    started.wait();

    StartTest("Trigger trim");
    WaitForPlaybackStopped stopped;
    WaitForPlaybackStarted startedAgain;
    pause(200);
    TimelineKeyPress('s');
    stopped.wait();
    startedAgain.wait();

    ASSERT_SELECTION({ VideoClip(0,4), AudioClip(0,4) });
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLength() + VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    ASSERT_EQUALS(VideoClip(0,6)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));

    StartTest("Stop playback");
    WaitForPlaybackStopped stoppedAgain;
    TimelineKeyPress(' ');
    stoppedAgain.wait();
    WaitForIdle;
}

} // namespace
