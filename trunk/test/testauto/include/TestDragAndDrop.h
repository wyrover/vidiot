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

#ifndef TEST_DRAGANDDROP_H
#define TEST_DRAGANDDROP_H

#include "TestAuto.h"

namespace test
{
class TestDragAndDrop : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestDragAndDrop>
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

    /// When multiple clips are selected, holding mouse down on a selected clip
    /// should not change the selection. Instead, dragging the mouse should then
    /// result in a drag operation that includes the 'clicked on' clip.
    void testStartDrag();

    void testDnd();

    /// Test drag and drop scenarios specific to having multiple tracks in the
    /// timeline.
    void testDndMultipleTracks();

    void testSnapping();

    /// See [#79]: Crash when dropping clip adjacent to a transition
    /// When a clipped was dropped such that it touches a transition, the
    /// clip besides the transition was deleed but the transition was not.
    /// Any attempt to access a frame of the transition caused a crash,
    /// since one of the adjacing clips was missing.
    void testDropAdjacentToTransition();

    /// A crossfade for which one of the sides has length 0 requires extra 
    /// handling when doing actions exactly along the 0 length side.
    void testDragZeroLengthSideOfTransition();

    /// A crossfade for which one of the sides has length 0 requires extra 
    /// handling when doing actions exactly along the 0 length side.
    void testDropZeroLengthSideOfTransition();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif