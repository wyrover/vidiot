#ifndef TEST_SAVING_AND_LOADING_H
#define TEST_SAVING_AND_LOADING_H

#include "FixtureProject.h"
#include "SuiteCreator.h"

namespace test {

class TestSavingAndLoading : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestSavingAndLoading>
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

    void testSaveAndLoad();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};

} // namespace

using namespace test;

#endif // TEST_SAVING_AND_LOADING_H