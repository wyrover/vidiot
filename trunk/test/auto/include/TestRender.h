#ifndef TEST_ONCE_H
#define TEST_ONCE_H

namespace test
{

class TestRender : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestRender>
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

    void testChangeRenderSettings();
    void testRendering();
    void testRenderingCodecs();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_ONCE_H