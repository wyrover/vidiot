#ifndef TEST_TIMELINE_H
#define TEST_TIMELINE_H

#include <list>
#include <wx/filename.h>
#include <boost/shared_ptr.hpp>
#include "FixtureProject.h"
#include "SuiteCreator.h"

namespace model {
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;
}

namespace gui { namespace timeline {
class Timeline;
}}

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
    void testDnd();

    /// Test the handling of transitions when doing drag and drop operations
    /// with clips.
    void testDnDTransition();

    /// Test the handling of two transitions that are adjacent. This means the
    /// first transition is a fade-out and the second is a fade-in type transition.
    void testAdjacentTransitions();

    /// The undo scenario at end was difficult to fix. It was caused by using Timeline as a
    /// identifying member for AClipEdit commands. Since the undo included undo'ing the creation
    /// of the timeline, the timeline was no longer a good identifier. Therefore, these commands
    /// now contain SequencePtr as identifier.
    ///
    /// This test also tests that when only one of the clips in a transition is moved, the
    /// transition is removed after dropping that clip.
    void testUndo();

    /// Test the following for In, Out, InAndOut transition:
    /// - Move all related clips, resulting in transition move also
    /// - Scrub transition
    /// - Playback transition
    void testTransition();

    /// Test the splitting of the sequence at the position of the cursor
    void testSplitting();

    /// The following scenario caused pixel to pts conversion errors; after
    /// one DND operation (with shifting) the timeline positioning did not work anymore.
    /// The error caused LeftPixel() to return not the left most pixel of a clip,
    /// but the rightmost pixel of the previous clip.
    void testAbortDrag();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif // TEST_TIMELINE_H