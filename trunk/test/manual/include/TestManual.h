#ifndef TEST_MANUAL_H
#define TEST_MANUAL_H

#include "FixtureProject.h"
#include "SuiteCreator.h"

namespace test
{

class TestManual : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestManual>
{
public:

    TestManual();

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