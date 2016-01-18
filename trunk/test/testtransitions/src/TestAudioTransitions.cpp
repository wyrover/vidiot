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

#include "TestAudioTransitions.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestAudioTransitions::setUp()
{
    mProjectFixture.init();
}

void TestAudioTransitions::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestAudioTransitions::testAudioTransitions()
{
    StartTestSuite();
    TimelineZoomIn(3);
    {
        MakeInOutTransitionAfterClip preparation(1,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        TimelineLeftClick(Center(AudioClip(0, 2)));
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
    {
        MakeInTransitionAfterClip preparation(1,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        TimelineLeftClick(Center(AudioClip(0, 2)));
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
    {
        MakeOutTransitionAfterClip preparation(1,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        TimelineLeftClick(Center(AudioClip(0, 2)));
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
    {
        MakeInOutTransitionAfterClip preparationAudio(1,true);
        MakeInOutTransitionAfterClip preparationVideo(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        TimelineLeftClick(Center(AudioClip(0, 2)));
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
}

} // namespace
