#ifndef HELPER_TEST_SUITE_H
#define HELPER_TEST_SUITE_H

#include <list>
#include <string>
#include <boost/optional.hpp>
#include "FixtureConfig.h"

namespace test {

/// Various helpers exist to simplify working with the large number of test suites, and the fact
/// that it can be difficult to debug (since the test application depends on mouse positioning
/// and keyboard states)
///
/// Run only one specific test:
/// RUNONLY(testTrimming);
/// Note 1: This must be placed just above the method's definition.
/// Note 2: For this to work all test suites must have the same name as the file they're in
///         (Thus, if this is part of TestTimeline.cpp, the class should be named TestTimeline).
///
/// Run a test suite without starting the full GUI:
/// RUNWITHOUTGUI(testConversions);
/// Same notes as for RUNONLY apply.
///
/// Run only the tests AFTER (and including) one specific test:
/// RUNFROM(testTrimming);
/// Same notes as for RUNONLY apply.
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
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    HelperTestSuite();
    static HelperTestSuite& get();

    //////////////////////////////////////////////////////////////////////////
    // TEST CONFIGURATION
    //////////////////////////////////////////////////////////////////////////

    /// Configure the test identified by file and name to be the first test that is ran.
    /// \param file name of file in which the test resides (NOTE: path and extensions will be stripped in this method to extract the class name)
    /// \param test name of the method that is executed
    /// \return 1 (required to avoid being optimized out of executable)
    int runFrom(const char* file, const char* test);

    /// Configure the test identified by file and name to be the only test that is ran.
    /// \param file name of file in which the test resides (NOTE: path and extensions will be stripped in this method to extract the class name)
    /// \param test name of the method that is executed
    /// \return 1 (required to avoid being optimized out of executable)
    int runOnly(const char* file, const char* test);

    /// Configure the test identified by file and name to be ran without starting the GUI.
    /// \param file name of file in which the test resides (NOTE: path and extensions will be stripped in this method to extract the class name)
    /// \param test name of the method that is executed
    /// \return 1 (required to avoid being optimized out of executable)
    int runWithoutGui(const char* file, const char* test);

    /// \return true if all tests are being run.
    /// \see runOnly
    bool allTestsAreRunning();

    /// \return true if the current test (as indicated by CxxTest) runs as the only test.
    /// \see runOnly
    bool currentTestRunsStandAlone();

    /// \return true if the current test (as indicated by CxxTest) may be ran.
    /// A test is ran if
    /// - no test has been configured as 'runonly', OR
    /// - the current CxxTest (identified via it's CxxTest name) is the test configured as 'runonly'
    /// \see runOnly
    bool currentTestIsEnabled();

    /// \return true if the current test (as indicated by CxxTest) requires a GUI.
    /// \see runWithoutGui
    bool currentTestRequiresGui();

    /// Ran before each test
    /// \return false if the test must NOT run (used in the macro below)
    bool startTestSuite(const char* suite);

    /// Store the current test name. Used for logging and updating the window title.
    void setTest(const char* test);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static HelperTestSuite* sInstance;
    std::string mRunOnlySuite;
    std::string mRunFromSuite;
    std::list<std::string> mSuitesWithoutGui;
    wxString mCurrentSuiteName;
    boost::optional<std::string> mCurrentTestName;
    int mSuiteCount;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateTitle();
};

#define StartTestSuite() \
    if (!HelperTestSuite::get().startTestSuite(__FUNCTION__)) return; \
    FixtureConfig ConfigFixture; \
    if (HelperTestSuite::get().currentTestRequiresGui()) ConfigFixture.SetDefaults();

#define StartTest(expr) HelperTestSuite::get().setTest(expr); LOG_WARNING << "Test: " << expr

/// Place in same file as the test case which must be the first test case that is ran.
/// All test cases before this test case are skipped. The mentioned test case and all test
/// cases after that are executed.
/// Also: Only use one Suite in one set of .h/.cpp files and name it the same as the file (without .h/.cpp)
/// This may only be called ONCE.
#define RUNFROM(testname) int i = HelperTestSuite::get().runFrom(__FILE__,#testname)

/// Place in same file as the test case which must be ran in exclusive mode
/// Also: Only use one Suite in one set of .h/.cpp files and name it the same as the file (without .h/.cpp)
/// This may only be called ONCE.
#define RUNONLY(testname) int i = HelperTestSuite::get().runOnly(__FILE__,#testname)

/// Place in same file as the test case which must be ran without GUI
/// Also: Only use one Suite in one set of .h/.cpp files and name it the same as the file (without .h/.cpp)
#define RUNWITHOUTGUI(testname) int j ## __COUNTER__ = HelperTestSuite::get().runWithoutGui(__FILE__,#testname)

} // namespace

#endif // HELPER_TEST_SUITE_H