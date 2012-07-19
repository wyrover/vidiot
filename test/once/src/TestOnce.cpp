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