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
    void testTransition();
    void testDnd();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_TIMELINE_H