#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperTestSuite.h"
#include "Constants.h"
#include "HelperWindow.h"
#include "Convert.h"
#include "Layout.h"
#include "HelperModel.h"
#include "Details.h"
#include "DetailsClip.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"

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
    StartTest("Manual...");
    pause(60000 * 60); // One hour should be enough...
}
} // namespace