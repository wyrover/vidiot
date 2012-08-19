#pragma warning(disable:4996)

#include "SuiteCreator.h"

#include <boost/optional.hpp>
#include "Window.h"

#include "UtilLog.h"

namespace test {

wxString sSuite;
boost::optional<std::string> sTest(boost::none);
int sSuiteCount = 0;
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
    std::string path(file);
    std::string filename(path.substr(path.find_last_of('\\')+1));
    std::string filename_noext(filename.substr(0, filename.find_last_of('.')));
    sInstance->mSuite = filename_noext;
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
    std::string suitename(suite);
    sSuite = suitename.substr(suitename.find_last_of(':')+1);
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