#include "TestSavingAndLoading.h"

#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperWindow.h"
#include "Sequence.h"
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
    wxFileName dirpath(wxFileName::GetTempDir(), "");
    dirpath.AppendDir(randomString(20));
    ASSERT(!wxDirExists(dirpath.GetLongPath()));
    dirpath.Mkdir();
    ASSERT(wxDirExists(dirpath.GetLongPath()));
    dirpath.SetName("LoadSave");
    dirpath.SetExt("vid");
    wxString path(dirpath.GetFullPath());
    StartTest("Save document");
    gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(path);
    gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(path);
    waitForIdle();
    triggerMenu(wxID_CLOSE);
    StartTest("Load document");
    triggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoid the open dialog.
    waitForIdle();
    StartTest("TearDown");
    triggerMenu(wxID_CLOSE);
    bool removed = wxFileName::Rmdir( dirpath.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
    ASSERT(removed);
    // Todo test all types of codecs, and parameters...
}

} // namespace