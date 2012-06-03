#ifndef TEST_BLEND_H
#define TEST_BLEND_H

#include "FixtureProject.h"
#include "SuiteCreator.h"

namespace test
{

class TestComposition : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestComposition>
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

    void testBlend();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_BLEND_H