#include "TestComposition.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "Convert.h"
#include "Details.h"
#include "DetailsClip.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperModel.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "ProjectViewCreateSequence.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"

namespace test {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestComposition::setUp()
{
    mProjectFixture.init();
}

void TestComposition::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY("testBlend");
void TestComposition::testBlend()
{
    StartTestSuite();
    StartTest("Iasdfdsaf.");
}
} // namespace