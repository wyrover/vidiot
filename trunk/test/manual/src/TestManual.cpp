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

    // todo make 3 video tracks. put 3 clips in one 'combiner'. Bottommost frame was shown when scrubbing, and when playback after shown while scrubbing.
    // howvever when scrubbing to the empty clip just before that bottommost clip, and then playback, bottommost clip not shown!!!

    // todo verdeling bij audio clip details: grow sliders much more
    // todo test on machine without msvc, since two ms dlls are installed in bin/

    // todo no audio in rendered file...

    // todo make details drag to show hints during dragging (see trim...)
    pause(60000 * 60); // One hour should be enough...
}

} // namespace