#ifndef TEST_AUTO_FOLDER_H
#define TEST_AUTO_FOLDER_H

#include "SuiteCreator.h"

namespace test
{

class TestAutoFolder : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestAutoFolder>
{
public:
    void testWatch();
};

}
using namespace test;

#endif // TEST_AUTO_FOLDER_H