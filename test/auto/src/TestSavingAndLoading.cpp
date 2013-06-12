#include "TestSavingAndLoading.h"

#include "HelperApplication.h"
#include "HelperFileSystem.h"
#include "HelperTimeline.h"
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
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName dirpath = tempDir->getFileName();
    dirpath.SetName("LoadSave");
    dirpath.SetExt("vid");
    wxString path(dirpath.GetFullPath());
    StartTest("Save document");
    gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(path);
    gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(path);
    waitForIdle();
    triggerMenu(wxID_CLOSE);
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