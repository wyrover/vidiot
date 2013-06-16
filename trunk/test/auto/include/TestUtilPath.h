#ifndef TEST_UTIL_PATH_H
#define TEST_UTIL_PATH_H

namespace test
{

class TestUtilPath : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestUtilPath>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testEqualsAndIsParentOf();
};

}
using namespace test;

#endif // TEST_UTIL_PATH_H