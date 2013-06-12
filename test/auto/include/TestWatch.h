#ifndef TEST_WATCH_H
#define TEST_WATCH_H

#include "HelperFileSystem.h"

namespace test {

class TestWatch: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestWatch>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testRemoveWatchedSubFolder();

private:
    model::FolderPtr mRoot;
    RandomTempDirPtr mTempDir;
    RandomTempDirPtr mSubDir;
    RandomTempDirPtr mSubSubDir;
    wxFileName mTempDirName;
    wxFileName mSubDirName;
    wxFileName mSubSubDirName;
};

} // namespace

using namespace test;

#endif // TEST_WATCH_H