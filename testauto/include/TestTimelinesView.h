#ifndef TEST_TIMELINES_VIEW_H
#define TEST_TIMELINES_VIEW_H

#include <wx/menu.h>
#include <boost/shared_ptr.hpp>
#include "HelperProjectView.h"
#include "SuiteCreator.h"

namespace test
{

class TestTimelinesView : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestTimelinesView>
    ,   public HelperProjectView
{
public:
    void testSequenceMenu();
};

}
using namespace test;

#endif // TEST_TIMELINES_VIEW_H