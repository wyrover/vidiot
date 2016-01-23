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

#include "Test.h"

namespace test
{

class TestDetailsClip : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestDetailsClip>
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

    void testChangeLength();

    // Due to the use of trim operations on both sides of the clip for changing the
    // clip's size, some buttons can be enabled although changing the clip's length
    // to that size is not allowed. However, this is only detected after the first 
    // trim has been executed.
    // Was bug once, resulting in crash: the 2.0s button was enabled because 
    // DetailsClip used only the selected clip (and not its link) for determining
    // the size bounds.
    void testChangeLengthTooMuch();

    void testChangeLengthOfTransition();

    /// After selecting a clip, then creating a fade in to the clip, and then immediately pressing
    /// one of the length buttons still available caused a crash when applying the trim. This was
    /// caused by the fact that the clip (as seen in the DetailsClip class) was no longer part of
    /// the track because of the create transition (particularly, because a trim was executed on
    /// the clip to make room for the transition). In fact, the details view showed the wrong clip
    /// at that point (it still showed info on the 'pre-create-transition' clip).
    void testChangeLengthAfterCreatingTransition();

    void testTransform();

    void testChangeVolume();

    /// See [#210]
    /// Select audio clip, then start playback.
    /// Change clip (via details pane) during playback causes crash.
    void testEditClipDuringPlayback();

    /// See [#252]
    /// After a clip is moved around, its 'details view' should be re-initialized.
    /// For example, after the move certain length buttons become (dis)abled because
    /// of the new clip position.
    void testClipUpdatedAfterDragAndDrop();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;
