#include "TestWatch.h"

#include <wx/filefn.h>
#include "File.h"
#include "Folder.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "Sequence.h"
#include "UtilPath.h"
#include "Watcher.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestWatch::testRemoveWatchedSubFolder()
{
    StartTestSuite();

    mRoot = createProject();
    ASSERT(mRoot);
    model::IPaths InputFiles = getListOfInputFiles();

    auto setup = [this]() -> model::FolderPtr
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
        ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),1); // Only the topmost autofolder is being watched
        return autofolder;
    };

    {
        StartTest("Remove autofolder/mSubDir/mSubSubDir");
        FolderHelper rootFolder(setup());

        mSubSubDir.reset(); // Delete
        WaitForChildCount(mRoot, 3);

        ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),1); // Only the topmost autofolder is being watched
        ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
        ASSERT_EQUALS(mRoot->find(util::path::toPath(mSubDirName)).size(),0); // Subdir name is relative to parent
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),1); // Subdir name is relative to parent
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),0); // Subsubdir is not present anymore
    }
    {
        StartTest("Remove autofolder/mSubDir");
        FolderHelper rootFolder(setup());

        mSubDir.reset(); // Delete
        WaitForChildCount(mRoot, 2);

        ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),1); // Only the topmost autofolder is being watched
        ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),0); // Subdir is not present anymore
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),0); // Subsubdir is not present anymore
        // todo fix wx error: see debug logging (for failed to create tooltip)
    }
    {
        StartTest("Add supported and valid file on disk");
        FolderHelper rootFolder(setup());

        wxString aviFileName = InputFiles.front()->getPath().GetLongPath();
        wxFileName filepath(mTempDirName.GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
        ASSERT(copyok);
        WaitForChildCount(mRoot, 5);

        ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),1); // Only the topmost autofolder is being watched
        ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubDirName)).size(),1); // Subdir is not present anymore
        ASSERT_EQUALS(mRoot->find(util::path::toName(mSubSubDirName)).size(),1); // Subsubdir is not present anymore
        ASSERT_EQUALS(mRoot->find(util::path::toName(filepath)).size(),1); // File is present

        StartTest("Remove file on disk");
        bool removeok = wxRemoveFile(filepath.GetFullPath());
        ASSERT(removeok);
        WaitForChildCount(mRoot, 4);
    }
    {
        StartTest("Add supported but not valid file on disk");
        FolderHelper rootFolder(setup());

        wxFileName filepath(mTempDirName.GetLongPath(),"invalid","avi");
        wxFFile aviFile1( filepath.GetLongPath(), "w" );
        aviFile1.Write( "Dummy Contents", wxFile::read_write );
        aviFile1.Close();

        // Add another file to make sure we've waited long enough
        wxString aviFileName = InputFiles.front()->getPath().GetLongPath();
        wxFileName filepath2(mTempDirName.GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath2.GetLongPath(), false );
        ASSERT(copyok);
        WaitForChildCount(mRoot, 5); // Only one of the two files is added
    }

    ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),0); // Nothing is being watched
    mRoot.reset();

    // todo test remove root watched dir and test
}

} // namespace