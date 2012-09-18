#ifndef HELPER_TEST_SUITE_H
#define HELPER_TEST_SUITE_H

#include <list>
#include <string>
#include <boost/optional.hpp>

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
/// Break execution and give the focus back to the tests afterwards:
/// BREAK();
///
/// Dump the timeline contents:
/// getTimeline().getDump().dump();

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

    /// \return true if the current test (as indicated by CxxTest) may be ran.
    /// A test is ran if
    /// - no test has been configured as 'runonly', OR
    /// - the current CxxTest (identified via it's CxxTest name) is the test configured as 'runonly'
    /// \see runOnly
    bool currentTestIsEnabled();

    /// \return true if the current test (as indicated by CxxTest) requires a GUI.
    /// \see runWithoutGui
    bool currentTestRequiresGui();

    /// Store the current test suite name. Used for logging and updating the window title.
    void setSuite(const char* suite);

    /// Store the current test name. Used for logging and updating the window title.
    void setTest(const char* test);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static HelperTestSuite* sInstance;
    std::string mRunOnlySuite;
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
    if (!HelperTestSuite::get().currentTestIsEnabled()) return; \
    HelperTestSuite::get().setSuite(__FUNCTION__); \
    LOG_WARNING << "Suite start: " << __FUNCTION__;

#define StartTest(expr) HelperTestSuite::get().setTest(expr); LOG_WARNING << "Test: " << expr

/// Place in same file as the test case which must be ran in exclusive mode
/// Also: Only use one Suite in one set of .h/.cpp files and name it the same as the file (without .h/.cpp)
/// This may only be called ONCE.
#define RUNONLY(testname) int i = HelperTestSuite::get().runOnly(__FILE__,#testname)

/// Place in same file as the test case which must be ran without GUI
/// Also: Only use one Suite in one set of .h/.cpp files and name it the same as the file (without .h/.cpp)
//#define UNIQUEVARNAME 0
#define RUNWITHOUTGUI(testname) int j ## __COUNTER__ = HelperTestSuite::get().runWithoutGui(__FILE__,#testname)
//; UNIQUEVARNAME = UNIQUEVARNAME ## j

} // namespace

#endif // HELPER_TEST_SUITE_H