#include "TestOnce.h"

#include <boost/make_shared.hpp>
#include "HelperConfig.h"
#include "Worker.h"
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Sequence.h"
#include "HelperWindow.h"
#include "HelperTestSuite.h"
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

RUNONLY(testOnce);

void TestOnce::testOnce()
{
    // BREAK();
    // ConfigFixture.SnapToAll(true);
    // LOG_ERROR << dump(getSequence());

    StartTestSuite();

    pause(600000);
}

} // namespace