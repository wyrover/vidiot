#ifndef TEST_INTERVALS_H
#define TEST_INTERVALS_H

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
class TestIntervals : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestIntervals>
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

    /// Test removing all intervals that are selected
    void testRemoveSelectedIntervals();

    /// Test removing all intervals that are not selected
    void testRemoveUnselectedIntervals();

    /// Test removing empty space between clips
    void testRemoveEmptyIntervals();

    /// Test removing empty space between clips, with multiple tracks, and with not precisely positioned clips
    void testRemoveEmptyIntervalsWithOffset();

    /// Test removing one empty area
    void testRemoveOneEmptyInterval();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif // TEST_INTERVALS_H