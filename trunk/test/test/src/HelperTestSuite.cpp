#pragma warning(disable:4996)

#include "HelperTestSuite.h"
#include <cxxtest/TestSuite.h>
#include "UtilList.h"
#include "UtilLog.h"
#include "Window.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// LOCAL HELPER METHODS
//////////////////////////////////////////////////////////////////////////

/// Convert a filename to a class name. Any slashes (path prefixes) are stripped, as well
/// as a file extension. If path does not contain slashes or an extension (dot), then
/// that's ok as well.
std::string convertToClassname(std::string path)
{
    size_t lastSlash = path.find_last_of('\\');
    size_t beginPositionForStrippingPath = (lastSlash == std::string::npos) ? 0 : lastSlash + 1; // NOTE: npos equals -1 already, but this seems better maintainable.
    std::string filename(path.substr(beginPositionForStrippingPath));
    size_t lastDot = filename.find_last_of('.');
    if (lastDot == std::string::npos)
    {
        return filename;
    }
    return filename.substr(0, lastDot);
}

std::string makeFullTestName(std::string file, std::string test)
{
    return convertToClassname(file) + "-" + test;
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
    , mRunFromSuite("")
    , mSuitesWithoutGui()
    , mCurrentSuiteName()
    , mCurrentTestName(boost::none)
    , mSuiteCount(0)
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CONFIGURATION
//////////////////////////////////////////////////////////////////////////

bool HelperTestSuite::allTestsAreRunning()
{
    return mRunOnlySuite.compare("") == 0;
}

bool HelperTestSuite::currentTestRunsStandAlone()
{
    return mRunOnlySuite.compare(currentCxxTest()) == 0;
}

bool HelperTestSuite::currentTestIsEnabled()
{
    if (mRunFromSuite.compare("") != 0)
    {
        if (mRunFromSuite.compare(currentCxxTest()) == 0)
        {
            // This is the first test that must run. Reset the var to enable subsequent tests to be ran.
            mRunFromSuite = "";
        }
        else
        {
            return false;
        }
    }
    return allTestsAreRunning() || currentTestRunsStandAlone();
}

bool HelperTestSuite::currentTestRequiresGui()
{
    return
        currentTestIsEnabled() &&
        !UtilList<std::string>(mSuitesWithoutGui).hasElement(currentCxxTest());
}

int HelperTestSuite::runFrom(const char* file, const char* test)
{
    ASSERT_ZERO( mRunFromSuite.compare("")); // Only one test may be 'run only' or 'run from' at a time
    ASSERT_ZERO( mRunOnlySuite.compare("")); // Only one test may be 'run only' or 'run from' at a time
    mRunFromSuite = makeFullTestName(file,test);
    return 1;
};

int HelperTestSuite::runOnly(const char* file, const char* test)
{
    ASSERT_ZERO( mRunFromSuite.compare("")); // Only one test may be 'run only' or 'run from' at a time
    ASSERT_ZERO( mRunOnlySuite.compare("")); // Only one test may be 'run only' or 'run from' at a time
    mRunOnlySuite = makeFullTestName(file,test);
    return 1;
};

int HelperTestSuite::runWithoutGui(const char* file, const char* test)
{
    mSuitesWithoutGui.push_back(makeFullTestName(file,test));
    return 1;
}

bool HelperTestSuite::startTestSuite(const char* suite)
{
    mSuiteCount++;
    if (!currentTestIsEnabled()) return false;
    std::string suitename(suite);
    mCurrentSuiteName = suitename.substr(suitename.find_last_of(':')+1);
    mCurrentSuiteName.Replace("test","Test",false);
    mCurrentTestName.reset();
    LOG_ERROR << "Suite start: " << suite;
    updateTitle();
    return true;
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