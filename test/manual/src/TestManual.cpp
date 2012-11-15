#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "Dialog.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperModel.h"
#include "HelperTestSuite.h"
#include "HelperTimelinesView.h"
#include "Layout.h"
#include "Logging.h"
#include "Sequence.h"
#include "UtilLog.h"
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
    StartTestSuite();
    StartTest("Manual...");
    LOG_ERROR << dump(getSequence());
    gui::Dialog::get().setSaveFile("D:/input.avi");
    pause(60000 * 60); // One hour should be enough...
}

} // namespace