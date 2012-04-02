#pragma warning(disable:4996)

#include "SuiteCreator.h"

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include "Window.h"
#include "UtilLog.h"

namespace test {

wxString sSuite;
boost::optional<std::string> sTest(boost::none);
int sSuiteCount = 1;
ISuite* ISuite::sInstance = 0;

ISuite::ISuite()
    : mSuite("")
    , mTest("")
{
}

int ISuite::runOnly(const char* file, const char* test)
{
    ASSERT_ZERO(ISuite::sInstance);
    ISuite::sInstance = new ISuite();
    sInstance->mTest = std::string(test);
    std::vector<std::string> fileparts;
    boost::split(fileparts, file, boost::is_any_of("\\"));
    boost::split(fileparts, fileparts.back(), boost::is_any_of("."));
    sInstance->mSuite = fileparts.front();
    return 1;
};

bool ISuite::currentTestIsDisabled()
{
    if (!sInstance)
    {
        // No test has been specified to run exclusively
        return false;
    }

    TS_ASSERT(sInstance->mSuite.compare("") != 0);
    TS_ASSERT(sInstance->mTest.compare("") != 0);

    std::string suite = CxxTest::TestTracker::tracker().test().suiteName();
    std::string test = CxxTest::TestTracker::tracker().test().testName();

    return ((sInstance->mSuite.compare(suite) != 0) || (sInstance->mTest.compare(test) != 0));
}

void updateTitle()
{
    wxString s;
    s << sSuiteCount << ": " << sSuite;
    if (sTest)
    {
        s << ": " << *sTest;
    }
    gui::Window::get().setAdditionalTitle(s);
}

void setSuite(const char* suite)
{
    std::vector<std::string> strs;
    boost::split(strs, suite, boost::is_any_of(":"));
    sSuite = strs.back();
    sSuite.Replace("test","Test",false);
    sTest.reset();
    updateTitle();
    sSuiteCount++;
}

void setTest(const char* test)
{
    sTest.reset(test);
    updateTitle();
}

} // namespace