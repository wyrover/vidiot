#ifndef TEST_AUTO_FOLDER_H
#define TEST_AUTO_FOLDER_H

#include "HelperProjectView.h"
#include "SuiteCreator.h"

namespace test
{

class TestAutoFolder : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestAutoFolder>
    ,   public HelperProjectView
{
public:
    void testWatch();
private:
};

}
using namespace test;

#endif // TEST_AUTO_FOLDER_H