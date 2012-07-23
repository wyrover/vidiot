#include "TestOnce.h"

#include <boost/make_shared.hpp>
#include "Render.h"
#include "RenderDialog.h"
#include "Sequence.h"
#include "HelperWindow.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "EmptyClip.h"
#include "HelperApplication.h"
#include "RenderDialog.h"
#include "VideoClip.h"
#include "Window.h"
#include "Dialog.h"
#include "UtilLog.h"
#include "ids.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestOnce::setUp()
{
    mProjectFixture.init();
}

void TestOnce::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

RUNONLY("testOnce");

void TestOnce::testOnce()
{
    // BREAK();
    // getTimeline().getDump().dump();
    // PrepareSnapping(true);
    StartTestSuite();

    // todo make into test loadsave
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
    gui::Window::get().GetDocumentManager()->CreateDocument(path,wxDOC_SILENT); // wxDOC_SILENT: no dialogs
    waitForIdle();
    StartTest("TearDown");
    triggerMenu(wxID_CLOSE);
    bool removed = wxFileName::Rmdir( dirpath.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
    ASSERT(removed);

    triggerMenu(ID_RENDERSETTINGS);
    waitForIdle();
    gui::Dialog::get().setSaveFile("D:/out.avi");
    pause(666666);
    ClickTopLeft(gui::RenderDialog::get().getFileButton());

    //model::render::RenderPtr render = boost::make_shared<model::render::Render>(getSequence());
    //gui::RenderDialog dialog(render);
    //dialog.ShowModal();
    //render->generate();
    pause(600000);
}

} // namespace