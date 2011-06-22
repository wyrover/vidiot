#ifndef TEST_UTIL_LIST_H
#define TEST_UTIL_LIST_H

#include "SuiteCreator.h"

namespace test {

class TestExample : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestExample>
{
public:
    void testAddRemove();
};

} // namespace

#endif //  TEST_UTIL_LIST_H