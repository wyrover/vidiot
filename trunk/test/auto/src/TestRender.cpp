#include "TestRender.h"

#include "Dialog.h"
#include "HelperApplication.h"
#include "HelperConfig.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ids.h"
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "Worker.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestRender::setUp()
{
    mProjectFixture.init();
}

void TestRender::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestRender::testChangeRenderSettings()
{
    StartTestSuite();
    {
        StartTest("If cancel is pressed, nothing is changed.");
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::RenderSettingsDialog::get().getCancelButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_EQUALS(*original,*current);
    }
    {
        StartTest("If apply is pressed, the sequence is changed (with a different video codec setting).");
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        model::render::RenderPtr original = getCurrentRenderSettings();
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::RenderSettingsDialog::get().getApplyButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
        ClickTopLeft(gui::RenderSettingsDialog::get().getCancelButton());
    }
    {
        StartTest("If apply is pressed, the sequence is changed (with a different audio codec setting).");
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        model::render::RenderPtr original = getCurrentRenderSettings();
        ClickBottomLeft(gui::RenderSettingsDialog::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::RenderSettingsDialog::get().getApplyButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
        ClickTopLeft(gui::RenderSettingsDialog::get().getCancelButton());
    }
    {
        StartTest("If OK is pressed, the sequence is changed and the dialog is closed.");
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        model::render::RenderPtr original = getCurrentRenderSettings();
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::RenderSettingsDialog::get().getOkButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
    }
}

//RUNONLY(testRendering);
void TestRender::testRendering()
{
    StartTestSuite();
    ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, 1); // Only render 1s
    {
        StartTest("Render");
        RandomTempDir tempdir;
        wxFileName path(tempdir.getFileName().GetLongPath(), "out", "avi");
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile(path.GetFullPath());
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        waitForIdle();
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::RenderSettingsDialog::get().getRenderButton());
        gui::Worker::get().waitUntilQueueEmpty();
        ASSERT(path.Exists());
    }
    {
        StartTest("Render each part of the sequence separately.");
        Click(Center(VideoClip(0,2)));
        ControlDown();
        Click(Center(VideoClip(0,4))); // Exclude clip 3 deliberately: include empty clips in the rendering
        Click(Center(VideoClip(0,5)));
        Click(Center(VideoClip(0,6)));
        ControlUp();
        Type(WXK_DELETE);
        RandomTempDir tempdir;
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        wxFileName fn(tempdir.getFileName().GetLongPath(), "out" ,"avi");
        gui::Dialog::get().setSaveFile(fn.GetLongPath());
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        ClickTopLeft(gui::RenderSettingsDialog::get().getRenderSeparationCheckBox(),wxPoint(4,4));
        ClickTopLeft(gui::RenderSettingsDialog::get().getRenderButton());
        gui::Worker::get().waitUntilQueueEmpty();
        for (int i = 1; i <= 3; ++i)
        {
            wxFileName f(tempdir.getFileName().GetLongPath(), wxString::Format("out_%d",i), "avi");
            ASSERT(f.Exists());
        }
    }
}

} // namespace