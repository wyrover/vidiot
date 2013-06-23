#ifndef TEST_EXCEPTIONS_H
#define TEST_EXCEPTIONS_H

#include "HelperFileSystem.h"

namespace test {

class TestExceptions: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestExceptions>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testRemovedFileInSequence();

    void testRemovedFileInSequenceBeforeOpening();

    void testRemovedFileInProjectViewBeforeOpening();

    void testRemovedFolderInProjectViewBeforeOpening();
};

} // namespace

using namespace test;

#endif // TEST_EXCEPTIONS_H