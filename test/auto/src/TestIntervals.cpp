#include "TestIntervals.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "EmptyClip.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperWindow.h"
#include "ids.h"
#include "VideoClip.h"
#include "VideoTrack.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestIntervals::setUp()
{
    mProjectFixture.init();
}

void TestIntervals::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY(testRemoveSelectedIntervals);
void TestIntervals::testRemoveSelectedIntervals()
{
    StartTestSuite();
    ConfigFixture.SnapToClips(true);
    Zoom Level(2);

    StartTest("Make an interval from left to right and click 'delete all marked intervals'");
    ToggleInterval(HCenter(VideoClip(0,1)), HCenter(VideoClip(0,2)));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    pts video1Adjustedlength = VideoClip(0,1)->getLength();
    pts video2Adjustedlength = VideoClip(0,2)->getLength();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval from right to left and click 'delete all marked intervals'");
    ToggleInterval(HCenter(VideoClip(0,2)), HCenter(VideoClip(0,1)));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), video1Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), video2Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval that completely deletes a clip");
    ToggleInterval(LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    Undo();
    Undo();
}

//RUNONLY(testRemoveUnselectedIntervals);
void TestIntervals::testRemoveUnselectedIntervals()
{
    StartTestSuite();
    ConfigFixture.SnapToClips(true);
    Zoom Level(2);

    StartTest("Make an interval from left to right and click 'delete all unmarked intervals'");
    ToggleInterval(10, HCenter(VideoClip(0,1))); // Can't use zero: the (test) code to trigger an interval first moves to the left a bit (and that would be < 0 in this case)
    ToggleInterval(HCenter(VideoClip(0,2)), RightPixel(VideoTrack(0)));
    triggerMenu(ID_DELETEUNMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    pts video1Adjustedlength = VideoClip(0,1)->getLength();
    pts video2Adjustedlength = VideoClip(0,2)->getLength();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    Undo();
    StartTest("Make an interval from right to left and click 'delete all unmarked intervals'");
    ToggleInterval(RightPixel(VideoTrack(0)), HCenter(VideoClip(0,2)));
    ToggleInterval(HCenter(VideoClip(0,1)), 0);
    triggerMenu(ID_DELETEUNMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), video1Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), video2Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    Undo();
    StartTest("Make an interval that completely deletes all but two clips");
    ToggleInterval(LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)));
    ToggleInterval(LeftPixel(VideoClip(0,3)), RightPixel(VideoClip(0,3)));
    triggerMenu(ID_DELETEUNMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoTrack(0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) + mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    Undo();
}

//RUNONLY(testRemoveEmptyIntervals);
void TestIntervals::testRemoveEmptyIntervals()
{
    StartTestSuite();
    {
        StartTest("Remove all empty space between all clips.");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Click(Center(VideoClip(0,0)));
        ControlDown();
        Click(Center(VideoClip(0,2)));
        Click(Center(VideoClip(0,3)));
        Click(Center(VideoClip(0,6)));
        ControlUp();
        Type(WXK_DELETE);
        ASSERT_VIDEOTRACK0(EmptyClip)(VideoClip)(     EmptyClip      )(VideoClip)(VideoClip)(EmptyClip);
        ASSERT_AUDIOTRACK0(EmptyClip)(AudioClip)(     EmptyClip      )(AudioClip)(AudioClip)(EmptyClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2) + mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,6));
        ASSERT_EQUALS(AudioClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,0));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,2) + mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,5));
        ASSERT_EQUALS(AudioClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,6));
        triggerMenu(ID_DELETEEMPTY);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoTrack(0)->getClips().size(), 3);
        ASSERT_EQUALS(AudioTrack(0)->getClips().size(), 3);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));
        ASSERT_EQUALS(AudioClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,5));
        Undo();
    }
}

//RUNONLY(testRemoveEmptyIntervalsWithOffset);
void TestIntervals::testRemoveEmptyIntervalsWithOffset()
{
    StartTestSuite();
    Zoom level(2);
    ConfigFixture.SnapToClips(true);
    triggerMenu(ID_ADDVIDEOTRACK);
    {
        StartTest("Remove empty intervals when clips are partially overlapping with the empty area");

        DragToTrack(1,VideoClip(0,3), model::IClipPtr());
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip           )(AudioClip);

        Drag(From(Center(VideoClip(1,1))).AlignLeft(LeftPixel(VideoClip(0,4))));
        ASSERT_VIDEOTRACK1(EmptyClip)                                 (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(           VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(AudioClip);

        DragToTrack(1,VideoClip(0,4), model::IClipPtr());
        ASSERT_VIDEOTRACK1(EmptyClip)                                 (           VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(                      EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(AudioClip);

        Drag(From(Center(VideoClip(1,1))).To(Center(VideoClip(1,1)) + wxPoint(-20,0)));
        ASSERT_VIDEOTRACK1(EmptyClip)                                 (           VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(                      EmptyClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip);

        Drag(From(Center(AudioClip(0,4))).To(RightCenter(AudioClip(0,2))));
        ASSERT_VIDEOTRACK1(EmptyClip)                                     (           VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(                      EmptyClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);

        pts gapLeft = VideoClip(1,1)->getLeftPts() - AudioClip(0,3)->getRightPts();
        pts gapRight = AudioClip(0,6)->getLength();
        pts expectedLength = VideoTrack(0)->getLength() - gapLeft - gapRight;

        triggerMenu(ID_DELETEEMPTY);

        ASSERT_EQUALS(VideoTrack(0)->getLength(), expectedLength);
        ASSERT_EQUALS(AudioTrack(0)->getLength(), expectedLength);
        ASSERT_EQUALS(VideoTrack(1)->getLength(), VideoClip(0,4)->getLeftPts());
        Undo(6);
    }
}

} // namespace