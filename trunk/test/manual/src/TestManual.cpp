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

    // todo crash: Select vairous clips after playback. Crash:
    //21-04-2013 11:06:27.125 WARNING t@1200 C:\Vidiot\vidiot_trunk\gui\src\VideoDisplay.cpp(378) gui::VideoDisplay::videoDisplayThread [paTime=6709207][mStartTime=6675565][mCurrentTime=33642][sleepTime=-2][nextFrameTime=33640][nextFrameTimeAdaptedForPlaybackSpeed=33640][mStartPts=31455][videoFrame->getPts()=32296]
    //21-04-2013 11:06:41.626 ASSERT  t@03ac C:\Vidiot\vidiot_trunk\model\src\File.cpp(128) model::File::getLength [ASSERT:mNumberOfFrames > 0][mNumberOfFrames=-1]
    //21-04-2013 11:06:41.626 ERROR   t@03ac C:\Vidiot\vidiot_trunk\gui\src\Application.cpp(246) gui::Application::onAssert
    //21-04-2013 11:06:41.627 ERROR   t@03ac C:\Vidiot\vidiot_trunk\gui\src\Dialog.cpp(230) gui::Dialog::getDebugReport [doexit=1]
    //21-04-2013 11:06:41.627 ERROR   t@03ac C:\Vidiot\vidiot_trunk\gui\src\Dialog.cpp(191) gui::generateDebugReport [doexit=1]

    // todo bug: installed version performs bad for playback of sequence (stutte%ring).
    // todo bug: made sequence 2010/01 jan. At a certain zoom change, the end clips were wrong. Playback was ok. UPDATE: this happens when maximizing the window!!!
}

} // namespace