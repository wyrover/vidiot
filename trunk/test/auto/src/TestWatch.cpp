#include "TestWatch.h"

#include <wx/filefn.h>
#include "File.h"
#include "Folder.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperWatcher.h"
#include "Sequence.h"
#include "UtilPath.h"
#include "Watcher.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestWatch::setUp()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled
    mRoot = createProject();
    ASSERT(mRoot);
    mInputFiles = getListOfInputFiles();
}

void TestWatch::tearDown()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled
    mRoot.reset();
    mTempDir.reset();
    mSubDir.reset();
    mSubSubDir.reset();
    mInputFiles.clear();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestWatch::testRemoveWatchedSubFolder()
{
    StartTestSuite();
    model::FolderPtr folder = setup();

    mSubDir.reset(); // Delete
    WaitForChildCount(mRoot, 2);

    ASSERT_WATCHED_PATHS_COUNT(1); // Only the topmost autofolder is being watched
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),0); // Subdir is not present anymore
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),0); // Subsubdir is not present anymore

    remove( folder );
    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
}

void TestWatch::testRemoveWatchedSubSubFolder()
{
    StartTestSuite();

    model::FolderPtr folder = setup();

    mSubSubDir.reset(); // Delete
    WaitForChildCount(mRoot, 3);

    ASSERT_WATCHED_PATHS_COUNT(1); // Only the topmost autofolder is being watched
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mSubDirName)).size(),0); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),1); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),0); // Subsubdir is not present anymore

    remove( folder );
    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
}

void TestWatch::testAddAndRemoveFileToWatchedAutoFolder()
{
    StartTestSuite();

    {
        model::FolderPtr folder = setup();

        StartTest("Add supported and valid file on disk");

        wxString aviFileName = mInputFiles.front()->getPath().GetLongPath();
        wxFileName filepath(mTempDirName.GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
        ASSERT(copyok);
        WaitForChildCount(mRoot, 5);

        ASSERT_WATCHED_PATHS_COUNT(1); // Only the topmost autofolder is being watched
        ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),1); // Subdir is not present anymore
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),1); // Subsubdir is not present anymore
        ASSERT_EQUALS(mRoot->find(util::path::toName(filepath)).size(),1); // File is present

        StartTest("Remove file on disk");
        bool removeok = wxRemoveFile(filepath.GetFullPath());
        ASSERT(removeok);
        WaitForChildCount(mRoot, 4);

        remove( folder );
        ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    }
    {
        StartTest("Add supported but not valid file on disk");
        model::FolderPtr folder = setup();

        wxFileName filepath(mTempDirName.GetLongPath(),"invalid","avi");
        wxFFile aviFile1( filepath.GetLongPath(), "w" );
        aviFile1.Write( "Dummy Contents", wxFile::read_write );
        aviFile1.Close();

        // Add another file to make sure we've waited long enough
        wxString aviFileName = mInputFiles.front()->getPath().GetLongPath();
        wxFileName filepath2(mTempDirName.GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath2.GetLongPath(), false );
        ASSERT(copyok);
        WaitForChildCount(mRoot, 5); // Only one of the two files is added

        remove( folder );
        ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    }
}

void TestWatch::testRemovedWatchedFolder()
{
    StartTestSuite();
    ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),0); // Nothing is being watched
    model::FolderPtr folder = setup();

    StartTest("Remove autofolder root dir");
    gui::Dialog::get().setConfirmation();
    mTempDir.reset();

    WaitForChildCount(mRoot, 1);

    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),0); // Full path for topmost autofolder
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mSubDirName)).size(),0); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),0); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),0); // Subsubdir is not present anymore
}

void TestWatch::testAddAndRemoveFileToWatchedNonAutoFolder()
{
    StartTestSuite();
    model::FolderPtr folder1 = addFolder( "TestFolder" );
    WaitForChildCount(mRoot, 2);

    StartTest("Add two files to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1_input(getTestFilesPath().GetFullPath(), "01.avi");
    wxFileName filepath2_input(getTestFilesPath().GetFullPath(), "03.avi");
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "01.avi");
    wxFileName filepath2(tempDir->getFileName().GetLongPath(), "03.avi");
    bool copyok = wxCopyFile( filepath1_input.GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);
    copyok = wxCopyFile( filepath2_input.GetLongPath(), filepath2.GetLongPath(), false );
    ASSERT(copyok);

    model::Files files1 = addFiles( boost::assign::list_of(filepath1)(filepath2), folder1 );
    WaitForChildCount(mRoot, 4);
    ASSERT_WATCHED_PATHS_COUNT(1);

    StartTest("Remove one file (on disk) that is child of a non-auto folder - one other file remains");
    gui::Dialog::get().setConfirmation();
    bool removeok = wxRemoveFile(filepath1.GetFullPath());
    ASSERT(removeok);
    WaitForChildCount(mRoot, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);

    StartTest("Remove last file (on disk) that is child of a non-auto folder - no other file remains");
    gui::Dialog::get().setConfirmation();
    removeok = wxRemoveFile(filepath2.GetFullPath());
    ASSERT(removeok);
    WaitForChildCount(mRoot, 2);
    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched anymore
}

void TestWatch::testRemoveProjectViewFolderContainingFileOnDisk()
{
    StartTestSuite();
    model::FolderPtr folder1 = addFolder( "TestFolder2" );
    WaitForChildCount(mRoot, 2);

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "01.avi");
    bool copyok = wxCopyFile( mInputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    model::Files files1 = addFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(mRoot, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);

    StartTest("Remove folder containing watched file");
    remove(folder1);
    ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),0); // Watch for the file must be removed

    StartTest("Add folder again (undo remove)");
    Undo();
    ASSERT_WATCHED_PATHS_COUNT(1); // Watch for the file must be added again

    StartTest("Remove the folder again (redo remove)");
    Redo();
    ASSERT_WATCHED_PATHS_COUNT(0); // Watch for the file must be removed
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::FolderPtr TestWatch::setup()
{
    ASSERT_ZERO(gui::Watcher::get().getWatchedPathsCount()); // Nothing is being watched
    mTempDir = RandomTempDir::generate();
    mSubDir = mTempDir->generateSubDir();
    mSubSubDir = mSubDir->generateSubDir();
    mTempDirName = mTempDir->getFileName();
    mSubDirName = mSubDir->getFileName();
    mSubSubDirName = mSubSubDir->getFileName();
    model::FolderPtr autofolder = addAutoFolder( mTempDir->getFileName() ); // Also waits for work
    WaitForChildCount(mRoot, 4);

    ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mSubDirName)).size(),0); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),1); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mSubSubDirName)).size(),0); // Subdir name is relative to parent
    ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),1); // Subdir name is relative to parent
    ASSERT_WATCHED_PATHS_COUNT(1); // Only the topmost autofolder is being watched
    return autofolder;
}

} // namespace