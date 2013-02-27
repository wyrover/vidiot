#ifndef TEST_TRANSITION_H
#define TEST_TRANSITION_H

#include "FixtureProject.h"
#include "SuiteCreator.h"

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

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif // TEST_TRANSITION_H