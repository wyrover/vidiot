#include "TestExceptions.h"

#include "File.h"
#include "Folder.h"
#include "HelperApplication.h"
#include "HelperProject.h"
#include "HelperProjectView.h"
#include "HelperWatcher.h"
#include "Sequence.h"
#include "UtilPath.h"
#include "Watcher.h"
#include <wx/filefn.h>

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestExceptions::testRemovedFileInSequence()
{
    StartTestSuite();
    model::FolderPtr root = createProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
    bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = addFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = addFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);
    model::SequencePtr sequence = createSequence(folder1);
    WaitForChildCount(root, 4);
    remove(folder1);
    ASSERT_WATCHED_PATHS_COUNT(0);
    triggerMenu(ID_CLOSESEQUENCE);

    tempDir.reset(); // Deletes the file (still used in the sequence) from disk

    // Open the sequence again (file missing from disk)
    getProjectView().select(boost::assign::list_of(folder1));
    getProjectView().GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,wxID_OPEN));
    waitForIdle();

    Play(10, 500);
}

void TestExceptions::testRemovedFileInSequenceBeforeOpening()
{
    StartTestSuite();
    model::FolderPtr root = createProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
    bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = addFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = addFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);
    model::SequencePtr sequence = createSequence(folder1);
    WaitForChildCount(root, 4);
    remove(folder1);
    ASSERT_WATCHED_PATHS_COUNT(0);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the file (still used in the sequence) from disk

    StartTest("Load document");
    triggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    waitForIdle();

    Play(10, 500);
}

void TestExceptions::testRemovedFileInProjectViewBeforeOpening()
{
    StartTestSuite();
    model::FolderPtr root = createProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
    bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = addFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = addFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the file (still used in the project view) from disk

    StartTest("Load document");
    gui::Dialog::get().setConfirmation(); // A confirmation for the dialog showing that the removed file is deleted from project
    triggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    waitForIdle();
}

void TestExceptions::testRemovedFolderInProjectViewBeforeOpening()
{
    StartTestSuite();
    model::FolderPtr root = createProject();
    ASSERT(root);

    RandomTempDirPtr tempDir = RandomTempDir::generate();
    RandomTempDirPtr subDir = tempDir->generateSubDir();
    RandomTempDirPtr subSubDir = subDir->generateSubDir();
    model::FolderPtr autofolder = addAutoFolder( tempDir->getFileName() ); // Also waits for work
    WaitForChildCount(root, 4);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the folder (still used in the project view) from disk

    StartTest("Load document");
    gui::Dialog::get().setConfirmation(); // A confirmation for the dialog showing that the removed file is deleted from project
    triggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    waitForIdle();//pause();
}

} // namespace