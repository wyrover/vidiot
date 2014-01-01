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

#include "TestSplitting.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "Config.h"
#include "CreateTransition.h"
#include "Details.h"
#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperPlayback.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelineTrim.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "Layout.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Transition.h"
#include "TrimClip.h"
#include "VideoClip.h"
#include "VideoDisplayEvent.h"
#include "VideoTrack.h"
#include "Zoom.h"

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
        PositionCursor(HCenter(VideoClip(0,2)));
        Type('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
    {
        PositionCursor(LeftPixel(VideoClip(0,2)));
        Type('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
    {
        PositionCursor(RightPixel(VideoClip(0,2)));
        Type('s');
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
    PositionCursor(HCenter(VideoClip(0,4)));
    WaitForPlaybackStarted started;
    Type(' ');
    started.wait();

    StartTest("Trigger trim");
    WaitForPlaybackStopped stopped;
    WaitForPlaybackStarted startedAgain;
    pause(200);
    Type('s');
    stopped.wait();
    startedAgain.wait();

    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLength() + VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    ASSERT_EQUALS(VideoClip(0,6)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));

    StartTest("Stop playback");
    WaitForPlaybackStopped stoppedAgain;
    Type(' ');
    stoppedAgain.wait();
    waitForIdle();
}

} // namespace