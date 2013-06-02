#ifndef HELPER_TEST_SUITE_H
#define HELPER_TEST_SUITE_H

#include <list>
#include <string>
#include <boost/optional.hpp>
#include "FixtureConfig.h"
#include "UtilSingleInstance.h"

namespace test {

/// Various helpers exist to simplify working with the large number of test suites, and the fact
/// that it can be difficult to debug (since the test application depends on mouse positioning
/// and keyboard states)
///
/// Break execution and give the focus back to the tests afterwards:
/// BREAK();
///
/// Dump the timeline contents:
/// #include "Logging.h"
/// LOG_ERROR << dump(getSequence());
///
/// Change the configuration: For tests with GUI must be done via the object
/// FixtureConfig ConfigFixture;

class HelperTestSuite
    : public SingleInstance<HelperTestSuite>
{
public:

    static std::string currentCxxTest();

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    HelperTestSuite();

    void readConfig();
    void onTestStopped();

    //////////////////////////////////////////////////////////////////////////
    // TEST CONFIGURATION
    //////////////////////////////////////////////////////////////////////////

    /// \return true if the current test (as indicated by CxxTest) may be ran.
    bool currentTestIsEnabled();

    /// \return true if the current test (as indicated by CxxTest) requires a visible window.
    bool currentTestRequiresWindow();

    /// Ran before each test
    /// \return false if the test must NOT run (used in the macro below)
    bool startTestSuite(const char* suite);

    /// Ran after a test is finished
    void testSuiteDone();

    /// Store the current test name. Used for logging and updating the window title.
    void setTest(const char* test);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::optional<std::string> mCurrentTestName;
    int mSuiteCount;

    wxString mRunOnly;
    wxString mRunFrom;
    wxString mRunCurrent;

    bool mFoundRunFromTest;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateTitle();
};

#define StartTestSuite() \
    if (!HelperTestSuite::get().startTestSuite(__FUNCTION__)) return; \
    FixtureConfig ConfigFixture; \
    if (HelperTestSuite::get().currentTestRequiresWindow()) ConfigFixture.SetDefaults();

#define StartTest(expr) HelperTestSuite::get().setTest(expr); LOG_WARNING << "Test: " << expr

} // namespace

#endif // HELPER_TEST_SUITE_H