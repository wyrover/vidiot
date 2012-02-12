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

void StartTestSuite();

#define StartTest(expr) LOG_WARNING << "Test: " << expr

void setTitle(const char* title);
} // namespace

#endif // TEST_SUITE_CREATOR_H