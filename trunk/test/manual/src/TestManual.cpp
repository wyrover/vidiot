#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "Convert.h"
#include "Details.h"
#include "DetailsClip.h"
#include "Dialog.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperConfig.h"
#include "HelperModel.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ids.h"
#include "ids.h"
#include "Layout.h"
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Sequence.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "Worker.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestManual::setUp()
{
    mProjectFixture.init();
}

void TestManual::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestManual::testManual()
{
    StartTestSuite();
    StartTest("Manual...");
    RandomTempDir tempdir;
    {
        StartTest("Render each part of the sequence separately.");
        ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, 1); // Only render 1s
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        wxFileName fn(tempdir.getFileName().GetLongPath(), "out" ,"avi");
        gui::Dialog::get().setSaveFile(fn.GetLongPath());
        ClickTopLeft(gui::RenderSettingsDialog::get().getFileButton());
        ClickTopLeft(gui::RenderSettingsDialog::get().getRenderSeparationCheckBox(),wxPoint(4,4));
        ClickTopLeft(gui::RenderSettingsDialog::get().getRenderButton());
        gui::Worker::get().waitUntilQueueEmpty();
        for (int i = 1; i <= NumberOfVideoClipsInTrack(); ++i)
        {
            wxFileName f(tempdir.getFileName().GetLongPath(), wxString::Format("out_%d",i), "avi");
            ASSERT(f.Exists());
        }
    }
    pause(60000 * 60); // One hour should be enough...
}

} // namespace