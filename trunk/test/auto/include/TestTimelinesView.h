#ifndef TEST_TIMELINES_VIEW_H
#define TEST_TIMELINES_VIEW_H

namespace test
{

class TestTimelinesView : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestTimelinesView>
{
public:
    void testSequenceMenu();
};

}
using namespace test;

#endif // TEST_TIMELINES_VIEW_H