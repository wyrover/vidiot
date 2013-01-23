#include "TestBugs.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureProject.h"
#include "HelperApplication.h"
#include "HelperConfig.h"
#include "HelperDetails.h"
#include "HelperTimelineDrag.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "Sequence.h"
#include "ids.h"
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

//RUNONLY(testLinkingErrorWhenDroppingOverBeginOfLinkedClip);
void TestBugs::testLinkingErrorWhenDroppingOverBeginOfLinkedClip()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);
    TrimLeft(VideoClip(0,4),40,false);
    Drag(From(Center(VideoClip(0,6))).To(wxPoint(RightPixel(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1)))));
    ASSERT_EQUALS(VideoClip(0,5)->getLink(),AudioClip(0,6));
    TrimLeft(VideoClip(0,5),10,false); // This caused an assert, because there was a problem with this clip (video(0,5)) link.
}

} // namespace