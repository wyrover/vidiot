#include "TestManual.h"

#include "Dialog.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperModel.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ProjectView.h"
#include "ids.h"
#include "Layout.h"
#include "File.h"
#include "Logging.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "UtilLogWxwidgets.h"
#include <boost/foreach.hpp>
#include <wx/uiaction.h>

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

    pause(60000 * 60); // One hour should be enough...

    // todo bug: installed version performs bad for playback of sequence (stutte%ring).
    // todo bug: made sequence 2010/01 jan. At a certain zoom change, the end clips were wrong. Playback was ok. UPDATE: this happens when maximizing the window!!!
    // todo bug: when zooming in: keep cursor position
}

} // namespace