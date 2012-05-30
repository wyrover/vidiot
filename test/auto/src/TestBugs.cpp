#include "TestBugs.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureProject.h"
#include "HelperTimeline.h"
#include "HelperDetails.h"
#include "HelperApplication.h"

namespace test {

void TestBugs::setUp()
{
}

void TestBugs::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestBugs::testVideoDecodingError()
{
    StartTestSuite();
    FixtureProject mProjectFixture;
    mProjectFixture.init();
    TrimRight(VideoClip(0,3),4);
    TrimRight(VideoClip(0,3),10);
    TrimRight(VideoClip(0,3),15);
    TrimRight(VideoClip(0,3),20);
    mProjectFixture.destroy();

}

void TestBugs::testHangupAfterResettingDetailsView()
{
    FixtureProject mProjectFixture;
    mProjectFixture.init();
    StartTestSuite();
    Click(VTopQuarterHCenter(VideoClip(0,2)));
    DeselectAllClips();
    mProjectFixture.destroy();
}

void TestBugs::testDetailsNotShownAfterMovingTimelineCursor()
{
    FixtureProject mProjectFixture;
    mProjectFixture.init();
    StartTestSuite();
    Click(Center(VideoClip(0,3)));
    PositionCursor(HCenter(VideoClip(0,3)));
    Click(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
}

} // namespace