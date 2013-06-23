#include "TestExceptions.h"

#include "File.h"
#include "Folder.h"
#include "HelperApplication.h"
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

} // namespace