#ifndef TEST_ONCE_H
#define TEST_ONCE_H

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

class TestOnce : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestOnce>
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

    void testOnce();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_ONCE_H