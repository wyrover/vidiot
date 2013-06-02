#include "TestBugs.h"

#include "HelperDetails.h"
#include "HelperTimeline.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "DetailsClip.h"
#include "IClip.h"
#include "Sequence.h"
#include "ids.h"

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

void TestBugs::testLinkingErrorWhenDroppingOverBeginOfLinkedClip()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);
    TrimLeft(VideoClip(0,4),40,false);
    Drag(From(Center(VideoClip(0,6))).To(wxPoint(RightPixel(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1)))));
    ASSERT_EQUALS(VideoClip(0,5)->getLink(),AudioClip(0,6));
    TrimLeft(VideoClip(0,5),10,false); // This caused an assert, because there was a problem with this clip (video(0,5)) link.
}

void TestBugs::testErrorInGetNextHandlingForEmptyClips()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDVIDEOTRACK);

    DragToTrack(1,VideoClip(0,5),model::IClipPtr());
    Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));

    DragToTrack(2,VideoClip(0,6),model::IClipPtr());
    Drag(From(Center(VideoClip(2,1))).AlignLeft(LeftPixel(VideoClip(1,1))));

    Click(Center(VideoClip(1,1)));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(6,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    TypeN(4,WXK_PAGEDOWN);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(4,WXK_PAGEDOWN);

    Click(Center(VideoClip(2,1)));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(6,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    TypeN(4,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(4,WXK_PAGEUP);

    PositionCursor(LeftPixel(VideoClip(2,1)) - 5);

    Play(LeftPixel(VideoClip(2,1)) - 3,2500); // Start before the clip, in the empty area. Due to a bug in 'getNext' handling for empty clips the clips after the empty area were not shown, or too late.

    Undo(9);
}

} // namespace