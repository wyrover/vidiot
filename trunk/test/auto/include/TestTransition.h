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

#ifndef TEST_TRANSITION_H
#define TEST_TRANSITION_H

namespace test
{
class TestTransition : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestTransition>
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

    void testSelectionAndDeletion();

    void testDragAndDropOfOtherClips();

    void testDragAndDropOfClipsUnderTransition();

    /// Test the handling of two transitions that are adjacent. This means the
    /// first transition is a fade-out and the second is a fade-in type transition.
    void testAdjacentTransitions();

    /// This tests (for In, Out as well as In&Out transitions)
    /// - when deleting a transition, the related clip's lengths are adjusted
    ///   accordingly (so that it looks as if the transition is just removed,
    ///   without affecting these clips. which in fact are changed).
    /// - when selecting only the clips related to the transition but not the
    ///   transition itselves, the transition is also taken along when doing
    ///   a DND operation.
    /// - playback of transition
    /// - scrubbing over the transition
    /// - Undoing
    void testPlaybackAndScrubbing();

    /// Tests the trimming of the transition as well as trimming the clips related
    /// to the transition. Also tests that maximum/minimum bounds are honored when
    /// trimming.
    void testTrimmingClipsInTransition();

    /// Test the trimming of clips which are linked to clips which, in turn, are
    /// used in a transition. Also tests that minimum/maximum bounds are honored when
    /// trimming.
    void testTrimmingLinkedClips();

    /// Test the trimming of the transition itselves
    void testTrimmingTransition();

    void testAudioTransitions();

    void testCreateTransitionAfterLastClip();

    void testVideoTransitionTypes();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif // TEST_TRANSITION_H