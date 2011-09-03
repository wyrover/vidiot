#ifndef TEST_PROJECT_VIEW_H
#define TEST_PROJECT_VIEW_H

#include "FixtureProject.h"
#include "SuiteCreator.h"

namespace test {

class ProjectViewTests : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<ProjectViewTests>
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

    void testAdditionAndRemoval();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};

} // namespace

using namespace test;

#endif // TEST_PROJECT_VIEW_H