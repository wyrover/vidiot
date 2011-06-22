#ifndef TEST_SUITE_CREATOR_H
#define TEST_SUITE_CREATOR_H

#include <cxxtest/TestSuite.h>

namespace test {

template <typename TESTS>
class SuiteCreator
{
public:
    static TESTS *createSuite() { return new TESTS(); };
    static void destroySuite(TESTS *suite) { delete suite; };
};

} // namespace

#endif // TEST_SUITE_CREATOR_H
