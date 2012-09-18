#pragma warning(disable:4996)

#include "HelperTestSuite.h"
#include <cxxtest/TestSuite.h>
#include "Window.h"
#include "UtilList.h"
#include "UtilLog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// LOCAL HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::string makeFullTestName(std::string file, std::string test)
{
    return file + "-" + test;
}

std::string currentCxxTest()
{
    return makeFullTestName( CxxTest::TestTracker::tracker().test().suiteName(), CxxTest::TestTracker::tracker().test().testName() );
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

HelperTestSuite* HelperTestSuite::sInstance = 0;

// static
HelperTestSuite& HelperTestSuite::get()
{
    if (!sInstance)
    {
        sInstance = new HelperTestSuite();
    }
    return *sInstance;
}

HelperTestSuite::HelperTestSuite()
    : mRunOnlySuite("")
    , mSuitesWithoutGui()
    , mCurrentSuiteName()
    , mCurrentTestName(boost::none)
    , mSuiteCount(0)
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CONFIGURATION
//////////////////////////////////////////////////////////////////////////

bool HelperTestSuite::currentTestIsEnabled()
{
    return
        mRunOnlySuite.compare("") == 0 ||
        mRunOnlySuite.compare(currentCxxTest()) == 0;
}

bool HelperTestSuite::currentTestRequiresGui()
{
    return
        currentTestIsEnabled() &&
        !UtilList<std::string>(mSuitesWithoutGui).hasElement(currentCxxTest());
}

int HelperTestSuite::runOnly(const char* file, const char* test)
{
    std::string path(file);
    std::string filename(path.substr(path.find_last_of('\\')+1));
    std::string filename_noext(filename.substr(0, filename.find_last_of('.')));
    mRunOnlySuite = makeFullTestName(filename_noext,test);
    return 1;
};

int HelperTestSuite::runWithoutGui(const char* file, const char* test)
{
    mSuitesWithoutGui.push_back(std::string(file) + "-" + test);
    return 1;
}

void HelperTestSuite::setSuite(const char* suite)
{
    mSuiteCount++;
    std::string suitename(suite);
    mCurrentSuiteName = suitename.substr(suitename.find_last_of(':')+1);
    mCurrentSuiteName.Replace("test","Test",false);
    mCurrentTestName.reset();
    updateTitle();
}

void HelperTestSuite::setTest(const char* test)
{
    mCurrentTestName.reset(test);
    updateTitle();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void HelperTestSuite::updateTitle()
{
    if (!currentTestRequiresGui()) { return; }
    wxString s;
    s << mSuiteCount << ": " << mCurrentSuiteName;
    if (mCurrentTestName)
    {
        s << ": " << *mCurrentTestName;
    }
    gui::Window::get().setAdditionalTitle(s);
}

} // namespace