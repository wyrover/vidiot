#ifndef TEST_MANUAL_H
#define TEST_MANUAL_H

#include <list>
#include <wx/filename.h>
#include "FixtureProject.h"
#include "SuiteCreator.h"

namespace model {
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;
}

namespace test
{

class TestManual : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestManual>
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

    void testManual();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_MANUAL_H