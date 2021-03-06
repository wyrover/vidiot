// Copyright 2015-2016 Eric Raijmakers.
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

#pragma once

#include "Test.h"

namespace test
{

class TestAudioKeyFrames : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestAudioKeyFrames>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual void setUp();       ///< Called before each test.
    virtual void tearDown();    ///< Called after each test.

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testAddKeyFrames();

    void testRemoveKeyFrames();

    void testNavigation();

    void testSelectKeyFrameInTimeline();

    /// Verify that the default key frame gets the parameters from the last
    /// remaining key frame.
    void testRemoveLastKeyFrame();

    void testKeyFramesOnBeginAndEndOfClip();

    void testGetKeyFrameWithoutOffset();

    void testGetKeyFrameWithOffset();

    void testGetKeyFrameWithTransition();

    void testChangeClipSpeed();

    void testTrimAwayKeyFrames();

    void testBeginOffset();

    void testInTransition();

    void testOutTransition();

    void testMovingKeyFramePosition();

    void testEditingAfterMoving();

    void testAbortMovingKeyFrame();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;
