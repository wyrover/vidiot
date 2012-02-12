#ifndef TEST_SUITE_CREATOR_H
#define TEST_SUITE_CREATOR_H

#include <cxxtest/TestSuite.h>
#include "FixtureApplication.h"

namespace test {
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

#define StartTestSuite() setSuite(__FUNCTION__); LOG_WARNING << "Suite start: " << __FUNCTION__
#define StartTest(expr) setTest(expr); LOG_WARNING << "Test: " << expr

void setSuite(const char* suite);
void setTest(const char* test);

} // namespace

#endif // TEST_SUITE_CREATOR_H