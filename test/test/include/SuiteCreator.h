#ifndef TEST_SUITE_CREATOR_H
#define TEST_SUITE_CREATOR_H

#include <cxxtest/TestSuite.h>
#include "FixtureApplication.h"

namespace test {

/// Various helpers exist to simplify working with the large number of test suites, and the fact
/// that it can be difficult to debug (since the test application depends on mouse positioning
/// and keyboard states)
///
/// Run only one specific test:
/// RUNONLY("testTrimming");
/// Note 1: This must be placed just above the method's definition.
/// Note 2: For this to work all test suites must have the same name as the file they're in
///         (Thus, if this is part of TestTimeline.cpp, the class should be named TestTimeline).
///
/// Break execution and give the focus back to the tests afterwards:
/// BREAK();
///
/// Dump the timeline contents:
/// getTimeline().getDump().dump();

class ISuite
{
public:
    ISuite();
    static bool currentTestIsDisabled();
    static int runOnly(const char* file, const char* test);
private:
    std::string mSuite;
    std::string mTest;
    static ISuite* sInstance;
};

template <typename TESTS>
class SuiteCreator
{
public:
    SuiteCreator()
    {
        FixtureGui::start(); // Include this 'do nothing' method to avoid FixtureGui being optimized out of the executable.
    }
    static TESTS *createSuite()
    {
        return new TESTS();
    };
    static void destroySuite(TESTS *suite)
    {
        delete suite;
    };
};

void setSuite(const char* suite);
void setTest(const char* test);

#define StartTestSuite() \
    if (ISuite::currentTestIsDisabled()) return; \
    setSuite(__FUNCTION__); \
    LOG_WARNING << "Suite start: " << __FUNCTION__;

#define StartTest(expr) \
    setTest(expr); LOG_WARNING << "Test: " << expr

/// Place in same file as the test case which must be ran in exclusive mode
/// Also: Only use one Suite in one set of .h/.cpp files and name it the same
///       as the file (without .h/.cpp)
#define RUNONLY(testname) \
    int i = ISuite::runOnly(__FILE__,testname);

} // namespace

#endif // TEST_SUITE_CREATOR_H