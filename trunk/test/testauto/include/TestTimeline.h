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

#ifndef TEST_TIMELINE_H
#define TEST_TIMELINE_H

#include "TestAuto.h"

namespace test
{
class TestTimeline : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestTimeline>
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

    void testSelection();

    void testDeletion();

    void testDeletionWithUnlinkedClips();

    /// The undo scenario at end was difficult to fix. It was caused by using Timeline as a
    /// identifying member for AClipEdit commands. Since the undo included undo'ing the creation
    /// of the timeline, the timeline was no longer a good identifier. Therefore, these commands
    /// now contain SequencePtr as identifier.
    ///
    /// This test also tests that when only one of the clips in a transition is moved, the
    /// transition is removed after dropping that clip.
    void testUndo();

    /// The following scenario caused pixel to pts conversion errors; after
    /// one DND operation (with shifting) the timeline positioning did not work anymore.
    /// The error caused LeftPixel() to return not the left most pixel of a clip,
    /// but the rightmost pixel of the previous clip.
    void testAbortDrag();

    /// Test moving up and down
    /// - audio/video divider
    /// - audio track divider
    /// - video track divider
    void testDividers();

    void testTrimming();

    /// Tests (shift)trimming restrictions imposed by clips in 'other' tracks
    void testTrimmingWithOtherTracks();

    /// Tests running with debug info enabled.
    void testShowDebugInfo();

    void testPositionCursor();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif