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

#pragma once

#include "TestAuto.h"

namespace test
{
class TestPlayback : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestPlayback>
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

    /// Move the cursor slightly before the end of the timeline.
    /// Then playback and check that playback stops automatically at the end
    /// of the timeline.
    void testPlaybackUntilEndOfSequence();

    /// During playback of a complex sequence frames need to be skipped
    /// sometimes (if decoding can't keep up with the required frame rate).
    /// Without skipping, nothing was shown during playback (video stopped,
    /// audio continued) and crashes could occur when stopping/starting
    /// playback.
    void testPlaybackComplexSequence();

    void testPlaybackWithDifferentSpeed();

    /// Caused crashes due to not properly (re)initializing playback.
    void testPlaybackAfterRangedPlayback();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;
