#ifndef TEST_MANUAL_H
#define TEST_MANUAL_H

#include <list>
#include <wx/filename.h>
#include "FixtureGui.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "SuiteCreator.h"

namespace model {
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;
}

namespace test
{

class TestManual : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<TestManual>
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

    void testManual();

private:

    wxFileName TestFilesPath;               ///< Contains the path to the test files
    model::IPaths InputFiles;               ///< Contains the filenames of the input files in the test directory

};

}
using namespace test;

#endif // TEST_MANUAL_H