#pragma warning(disable:4996)

#include "HelperTestSuite.h"
#include <cxxtest/TestSuite.h>
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilThread.h"
#include "Window.h"

namespace test {

std::list<std::string> SuitesWithoutGui = boost::assign::list_of
     ("TestConvert::testTimeConversions")("TestUtilPath::testEquals");

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
    return convertToClassname(file) + "::" + test;
}

// static
std::string HelperTestSuite::currentCxxTest()
{
    return makeFullTestName( CxxTest::TestTracker::tracker().test().suiteName(), CxxTest::TestTracker::tracker().test().testName() );
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

HelperTestSuite::HelperTestSuite()
    : mCurrentTestName(boost::none)
    , mSuiteCount(0)
    , mRunOnly("")
    , mRunFrom("")
    , mRunCurrent("")
    , mFoundRunFromTest(false)
{
}

void HelperTestSuite::readConfig()
{
    wxConfigBase::Set(new wxFileConfig(wxEmptyString, wxEmptyString, Config::getFileName()));
    wxString current;
    wxConfigBase::Get()->Read(Config::sPathTestRunOnly, &mRunOnly, "");
    wxConfigBase::Get()->Read(Config::sPathTestRunFrom, &mRunFrom, "");
    wxConfigBase::Get()->Read(Config::sPathTestRunCurrent, &current, "");
    wxConfigBase::Set(0);

    if (mRunFrom.IsSameAs(""))
    {
        mRunFrom = current; // If the previous run didn't succeed, continue at last started test.
    }
}

//////////////////////////////////////////////////////////////////////////
// TEST CONFIGURATION
//////////////////////////////////////////////////////////////////////////

bool HelperTestSuite::currentTestIsEnabled()
{
    wxString current = currentCxxTest();
    if (!mRunOnly.IsSameAs(""))
    {
        return current.IsSameAs(mRunOnly);
    }

    if (!mRunFrom.IsSameAs(""))
    {
        mFoundRunFromTest = mFoundRunFromTest || current.IsSameAs(mRunFrom);
        return mFoundRunFromTest;
    }

    return true;
}

bool HelperTestSuite::currentTestRequiresWindow()
{
    return
        currentTestIsEnabled() &&
        !UtilList<std::string>(SuitesWithoutGui).hasElement(currentCxxTest());
}

bool HelperTestSuite::startTestSuite(const char* suite)
{
    mSuiteCount++;
    if (!currentTestIsEnabled()) return false;
    mCurrentTestName.reset();
    VAR_ERROR(currentCxxTest());
    if (currentTestRequiresWindow())
    {
        updateTitle();
        waitForIdle();
        util::thread::RunInMain([this] { Config::WriteString( Config::sPathTestRunCurrent, currentCxxTest() ); }); // Set
    }
    return true;
}

void HelperTestSuite::testSuiteDone()
{
    if (mSuiteCount == CxxTest::TestTracker::tracker( ).world().numTotalTests())
    {
        util::thread::RunInMain([this] { Config::WriteString( Config::sPathTestRunCurrent, "" ); }); // Reset
    }
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
    if (!currentTestRequiresWindow()) { return; }
    wxString s;
    s << mSuiteCount << ": " << currentCxxTest();
    if (mCurrentTestName)
    {
        s << ": " << *mCurrentTestName;
    }
    gui::Window::get().setAdditionalTitle(s);
}

} // namespace