#ifndef TEST_CONVERT_H
#define TEST_CONVERT_H

namespace test
{

class TestConvert : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestConvert>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testTimeConversions();
};

}
using namespace test;

#endif // TEST_CONVERT_H