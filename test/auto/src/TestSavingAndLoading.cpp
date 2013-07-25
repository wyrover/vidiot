#include "TestSavingAndLoading.h"

#include "HelperApplication.h"
#include "HelperFileSystem.h"
#include "HelperProject.h"
#include "HelperTimeline.h"
#include "HelperTimelineTrim.h"
#include "HelperWindow.h"
#include "Project.h"
#include "Window.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::setUp()
{
    mProjectFixture.init();
}

void TestSavingAndLoading::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::testSaveAndLoad()
{
    StartTestSuite();
    StartTest("SetUp");

    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );
    wxFileName TestFilesPath = wxFileName(SOURCE_ROOT,"");
    TestFilesPath.AppendDir("test");
    TestFilesPath.AppendDir("filetypes_image");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    TestFilesPath.SetFullName("Laney -6th best amp.jpg");
    model::Files files1 = addFiles( boost::assign::list_of(TestFilesPath.GetFullPath()), folder1 );
    model::FilePtr imageFile = files1.front();

    DragFromProjectViewToTimeline( imageFile,  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(HCenter(VideoClip(0,4)), VCenter(VideoTrack(0))) );

    Click(Center(VideoClip(0,6)));
    Type(WXK_DELETE);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    StartTest("Load document");
    triggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    waitForIdle();
    StartTest("Trim clip");
    TrimLeft(VideoClip(0,1),20); // Known bug at some point: a crash due to improper initialization of File class members upon loading (mNumberOfFrames not initialized)
    Undo();
    model::Project::get().Modify(false); // Avoid 'save?' dialog
    StartTest("TearDown");
    triggerMenu(wxID_CLOSE);
}

} // namespace