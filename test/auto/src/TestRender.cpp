#include "TestRender.h"

#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ids.h"
#include "Dialog.h"
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Sequence.h"
#include "UtilLog.h"

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

model::render::RenderPtr getCurrentRenderSettings()
{
    return make_cloned<model::render::Render>(getSequence()->getRender());
}

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

} // namespace