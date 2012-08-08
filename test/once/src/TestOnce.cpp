#include "TestOnce.h"

#include <boost/make_shared.hpp>
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Sequence.h"
#include "HelperWindow.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "EmptyClip.h"
#include "HelperApplication.h"
#include "RenderSettingsDialog.h"
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

model::render::RenderPtr getCurrentRenderSettings()
{
    return make_cloned<model::render::Render>(getSequence()->getRender());
}

RUNONLY(testOnce);

void TestOnce::testOnce()
{
    // BREAK();
    // getTimeline().getDump().dump();
    // PrepareSnapping(true);
    StartTestSuite();

    // todo memory leaks bij testauto? Misschien dat daardoor de crashes wel komen?

    waitForIdle();
    {
        StartTest("If cancel is pressed, nothing is changed.");
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        waitForIdle();
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::RenderSettingsDialog::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
    }

    //todo test render->generate();
    pause(600000);
}

} // namespace