#include "TestPopupMenu.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "AudioView.h"
#include "Config.h"
#include "EmptyClip.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ids.h"
#include "Menu.h"
#include "Scrolling.h"
#include "SequenceView.h"
#include "TimescaleView.h"
#include "Transition.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoView.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestPopupMenu::setUp()
{
    mProjectFixture.init();
}

void TestPopupMenu::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestPopupMenu::testAddTransitions()
{
    StartTestSuite();
    Zoom level(2);
    ConfigFixture.SnapToClips(false);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    {
        StartTest("Add fade in");
        Move(Center(VideoClip(0,1)));
        ClickRight(false);
        Type('i'); // Fade &in
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(       AudioClip     )(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        Undo();
    }
    {
        StartTest("Add fade out");
        Move(Center(VideoClip(0,1)));
        ClickLeft(false); // To avoid seeing the next right click as a 'double' right click
        ClickRight(false);
        Type('o'); // Fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(      AudioClip      )(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        Undo();
    }
    {
        StartTest("Add crossfade to next clip");
        Move(Center(VideoClip(0,1)));
        ClickLeft(false);
        ClickRight(false);
        Type('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip      )(      AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength);
        Undo();
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Add crossfade to next clip");
        Move(Center(VideoClip(0,0)));
        ClickLeft(false);
        ClickRight(false);
        Type('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip      )(      AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength);
        Undo();
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Add fade in adjacent to fade out");
        Move(Center(VideoClip(0,1)));
        ClickRight(false);
        Type('i'); // Fade &in
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(       AudioClip     )(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        Move(Center(VideoClip(0,0)));
        ClickLeft(false); // To avoid seeing the next right click as a 'double' right click
        ClickRight(false);
        Type('o'); // Fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(Transition)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip            )(      AudioClip      );
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,1)->getRightPts(), AudioClip(0,0)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,2)->getLeftPts(), AudioClip(0,0)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,3)->getRightPts(), AudioClip(0,1)->getRightPts());
        Undo(2);
    }
}

void TestPopupMenu::testRemoveOneEmptyInterval()
{
    StartTestSuite();
    Zoom level(2);
    ConfigFixture.SnapToClips(false);
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);

    {
        StartTest("Remove empty intervals when clips are partially overlapping with the empty area");

        DragToTrack(1, VideoClip(0,3), AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip           )(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);

        Drag(From(Center(VideoClip(1,1))).MoveRight(50));
        Move(Center(VideoClip(0,3)));
        ASSERT_VIDEOTRACK1(EmptyClip)                           (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip           )(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                           (AudioClip);

        pts gap = VideoClip(1,1)->getLeftPts() - VideoClip(0,3)->getLeftPts();
        pts len1 = VideoTrack(1)->getLength();
        pts len0 = VideoTrack(0)->getLength();

        ClickRight(false);
        Type('e'); // Remove &empty space
        ASSERT_EQUALS(VideoTrack(1)->getLength(), len1 - gap);
        ASSERT_EQUALS(VideoTrack(0)->getLength(), len0 - gap);
        ASSERT_EQUALS(AudioTrack(0)->getLength(), len0 - gap);
        ASSERT_EQUALS(AudioTrack(1)->getLength(), len1 - gap);

        Undo(5);
    }
}

void TestPopupMenu::testAdvancedRightClickScenarios()
{
    StartTestSuite();
    Zoom level(2);
    ConfigFixture.SnapToClips(false);
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);
    // NOTE: Avoid the use of waitForIdle while the popup menu is being shown.
    //       Popup menu blocks the idle events causing long delays...
    {
        StartTest("Open popup menu twice");
        Move(Center(VideoClip(0,3)));
        ClickRight(false); // The menu is shown (started from state rightdown)
        pause(100);
        ASSERT(getTimeline().getMenuHandler().isPopupShown());
        wxUIActionSimulator().MouseMove(wxGetMouseState().GetPosition() - wxPoint(150,0));
        ClickRight(false); // The menu is shown (started from state scrolling)
        pause(100);
        ASSERT(getTimeline().getMenuHandler().isPopupShown());
        Type(WXK_ESCAPE);
        pause(100);
        ASSERT(!getTimeline().getMenuHandler().isPopupShown());
    }
    {
        StartTest("Open popup menu then start a scrolling via right mouse down");
        pixel length = getTimeline().getSequenceView().getSize().GetWidth();
        Drag(From(Center(VideoClip(0,4))).To(wxPoint(getTimeline().GetRect().GetRight() - 10, VCenter(VideoTrack(0))))); // extend the track to enable scrolling
        ASSERT_MORE_THAN(getTimeline().getSequenceView().getSize().GetWidth(),length);
        ASSERT_MORE_THAN(getTimeline().getSequenceView().getVideo().getSize().GetWidth(),length);
        ASSERT_MORE_THAN(getTimeline().getSequenceView().getAudio().getSize().GetWidth(),length);
        ASSERT_MORE_THAN(getTimeline().getSequenceView().getTimescale().getSize().GetWidth(),length);
        Move(Center(VideoClip(0,4)));
        ClickRight(false); // The menu is shown (started from state rightdown)
        pause(100);
        ASSERT(getTimeline().getMenuHandler().isPopupShown());
        ASSERT_ZERO(getTimeline().getScrolling().getOffset().x);
        ASSERT_ZERO(getTimeline().getScrolling().getOffset().y);
        wxUIActionSimulator().MouseMove(wxGetMouseState().GetPosition() - wxPoint(50,0));
        RightDown();
        waitForIdle();
        ASSERT(!getTimeline().getMenuHandler().isPopupShown());
        wxUIActionSimulator().MouseMove(wxGetMouseState().GetPosition() - wxPoint(150,0));
        waitForIdle();
        ASSERT_EQUALS(getTimeline().getScrolling().getOffset().x, 150);
        ASSERT_ZERO(getTimeline().getScrolling().getOffset().y);
        RightUp(false);
        pause(100);
        ASSERT(getTimeline().getMenuHandler().isPopupShown());
        wxUIActionSimulator().MouseMove(wxGetMouseState().GetPosition() - wxPoint(20,0));
        wxUIActionSimulator().MouseClick();
    }
}

} // namespace