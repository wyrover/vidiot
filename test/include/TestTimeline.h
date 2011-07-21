#ifndef TEST_TIMELINE_H
#define TEST_TIMELINE_H

#include <wx/menu.h>
#include <boost/shared_ptr.hpp>
#include "SuiteCreator.h"

namespace model {
    class Sequence;
    typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace gui { namespace timeline {
    class Timeline;
}}

namespace test
{

class TestTimeline : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestTimeline>
{
public:
    void testSelection();
    void testTransition();
    void testDnd();
};

}
using namespace test;

#endif // TEST_TIMELINE_H