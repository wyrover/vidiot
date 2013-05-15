#ifndef TEST_TRANSFORM_H
#define TEST_TRANSFORM_H

namespace test
{

class TestTransform : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestTransform>
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

    void testTransformViaDetailsView();

    void testTransformViaDetailsView_Boundaries();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_TRANSFORM_H