#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureGui.h"
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
#include "ids.h"
#include "Sequence.h"
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
    triggerMenu(ID_ADDVIDEOTRACK);
    Drag(Center(VideoClip(0,6)),wxPoint(HCenter(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1))));

    // todo known bug:
    //triggerMenu(ID_ADDVIDEOTRACK);
    //TrimLeft(VideoClip(0,4),40,false);
    //Drag(Center(VideoClip(0,6)),wxPoint(HCenter(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1))));
    //TrimLeft(VideoClip(0,5),10,false); // This caused an assert, because there was a problem with this clip (video(0,5)) link.

    pause(60000 * 60); // One hour should be enough...
}
} // namespace