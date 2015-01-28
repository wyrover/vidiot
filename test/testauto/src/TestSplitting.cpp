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
    MakeInOutTransitionAfterClip preparation(1);
    {
        TimelinePositionCursor(HCenter(VideoClip(0,2)));
        TimelineKeyPress('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
    {
        TimelinePositionCursor(LeftPixel(VideoClip(0,2)));
        TimelineKeyPress('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
    {
        TimelinePositionCursor(RightPixel(VideoClip(0,2)));
        TimelineKeyPress('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
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
