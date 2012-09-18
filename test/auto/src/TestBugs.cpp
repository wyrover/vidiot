#include "TestBugs.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureProject.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "UtilLog.h"

namespace test {

void TestBugs::setUp()
{
    mProjectFixture.init();
}

void TestBugs::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestBugs::testVideoDecodingError()
{
    StartTestSuite();
    TrimRight(VideoClip(0,3),4);
    TrimRight(VideoClip(0,3),10);
    TrimRight(VideoClip(0,3),15);
    TrimRight(VideoClip(0,3),20);
}

void TestBugs::testHangupAfterResettingDetailsView()
{
    StartTestSuite();
    Click(VTopQuarterHCenter(VideoClip(0,2)));
    DeselectAllClips();
}

void TestBugs::testDetailsNotShownAfterMovingTimelineCursor()
{
    StartTestSuite();
    Click(Center(VideoClip(0,3)));
    PositionCursor(HCenter(VideoClip(0,3)));
    Click(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
}

} // namespace