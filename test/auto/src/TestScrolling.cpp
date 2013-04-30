#include "TestScrolling.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestScrolling::setUp()
{
    mProjectFixture.init();
}

void TestScrolling::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY(testShowScrollbarWhenAddingClipAtEnd);
void TestScrolling::testShowScrollbarWhenAddingClipAtEnd()
{
    StartTestSuite();
    StartTest("Move clip beyond end of timeline. Scrollbar is shown.");
    ASSERT_EQUALS(getTimeline().GetClientSize(),getTimeline().GetVirtualSize()); // No scrollbars
    pixel length = getTimeline().getSequenceView().getSize().GetWidth();
    Drag(From(Center(VideoClip(0,4))).To(wxPoint(getTimeline().GetRect().GetRight() - 10, VCenter(VideoTrack(0))))); // extend the track to enable scrolling
    ASSERT_DIFFERS(getTimeline().GetClientSize(),getTimeline().GetVirtualSize()); // Scrollbars: Check that the scrolled area != physical area of widget
}

} // namespace