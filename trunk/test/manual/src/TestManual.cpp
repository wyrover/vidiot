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
//    pause(60000 * 60); // One hour should be enough...
}

} // namespace