#ifndef TEST_TIMELINES_VIEW_H
#define TEST_TIMELINES_VIEW_H

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

class TestTimelinesView : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestTimelinesView>
{
public:
    void testSequenceMenu();
private:
    static wxMenu* getSequenceMenu();
    static gui::timeline::Timeline* getTimeline( model::SequencePtr sequence );
};

}
using namespace test;

#endif // TEST_TIMELINES_VIEW_H