#ifndef TEST_TIMELINE_H
#define TEST_TIMELINE_H

#include <wx/menu.h>
#include <wx/uiaction.h>
#include <boost/shared_ptr.hpp>
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "SuiteCreator.h"

namespace model {
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;
}

namespace gui { namespace timeline {
class Timeline;
}}

namespace test
{

class TestTimeline : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestTimeline>
    ,   public HelperTimeline
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

    void testSelection();
    void testTransition();
    void testDnd();

private:

    wxFileName TestFilesPath;               ///< Contains the path to the test files
    model::IPaths InputFiles;               ///< Contains the filenames of the input files in the test directory
};

}
using namespace test;

#endif // TEST_TIMELINE_H