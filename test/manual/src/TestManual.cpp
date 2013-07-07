#include "TestManual.h"

#include "Dialog.h"
#include "DetailsClip.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperModel.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelineDrag.h"
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

TestManual::TestManual()
    :   mProjectFixture(false) // Set to true if the timeline must be focused initially
{
}

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

    // todo make help text
    // todo make audio composition
    // todo scale clip (reduce size), then put this clip in track2, then fade in this clip above another clip -> the black is also 'faded in', whereas it should have been 'empty'!
    // todo at end of clip edit, remove any empty clips which are the last clips of the track
    pause(60000 * 60); // One hour should be enough...
}

} // namespace