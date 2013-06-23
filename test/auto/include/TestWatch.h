#ifndef TEST_WATCH_H
#define TEST_WATCH_H

#include "HelperFileSystem.h"

namespace test {

class TestWatch: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestWatch>
{
public:

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

    void setUp();
    void tearDown();

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testRemoveWatchedSubSubFolder();
    void testRemoveWatchedSubFolder();
    void testRemovedWatchedFolder();
    void testAddAndRemoveFileToWatchedAutoFolder();
    void testAddAndRemoveFileToWatchedNonAutoFolder();
    void testRemoveProjectViewFolderContainingFileOnDisk();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::FolderPtr mRoot;
    RandomTempDirPtr mTempDir;
    RandomTempDirPtr mSubDir;
    RandomTempDirPtr mSubSubDir;
    wxFileName mTempDirName;
    wxFileName mSubDirName;
    wxFileName mSubSubDirName;
    model::IPaths mInputFiles;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    model::FolderPtr setup();
};

} // namespace

using namespace test;

#endif // TEST_WATCH_H