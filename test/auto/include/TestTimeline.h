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

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_TIMELINE_H