// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "TestUiTimelinePopupMenu.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiTimelinePopupMenu::setUp()
{
    mProjectFixture.init();
}

void TestUiTimelinePopupMenu::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiTimelinePopupMenu::testAddTransitions()
{
    StartTestSuite();
    TimelineZoomIn(6);
    ConfigFixture.SnapToClips(false);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    {
        StartTest("Add fade in");
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('i'); // Fade &in
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(       AudioClip     )(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        Undo();
    }
    {
        StartTest("Add fade out");
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // Fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(      AudioClip      )(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        Undo();
    }
    {
        StartTest("Add crossfade from previous clip");
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
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
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
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
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('i'); // Fade &in
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(       AudioClip     )(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        TimelineMove(Center(VideoClip(0,0)));
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('o'); // Fade &out
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

void TestUiTimelinePopupMenu::testDelete()
{
    StartTestSuite();
    {
        StartTest("Delete without shift");
        TimelineOpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('d');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip);
        Undo();
        TimelineLeftClick(Center(VideoClip(0,0)));
    }
    {
        StartTest("Delete with shift");
        TimelineOpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('t');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
        Undo();
    }
}

void TestUiTimelinePopupMenu::testRemoveOneEmptyInterval()
{
    StartTestSuite();
    TimelineZoomIn(2);
    ConfigFixture.SnapToClips(false);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);

    {
        StartTest("Remove empty intervals when clips are partially overlapping with the empty area");

        TimelineDragToTrack(1, VideoClip(0,3), AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip           )(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);

        TimelineDrag(From(Center(VideoClip(1,1))).MoveRight(50));
        ASSERT_VIDEOTRACK1(EmptyClip)                           (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip           )(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                           (AudioClip);

        pts gap = VideoClip(1,1)->getLeftPts() - VideoClip(0,3)->getLeftPts();
        pts len1 = VideoTrack(1)->getLength();
        pts len0 = VideoTrack(0)->getLength();

        TimelineOpenPopupMenuAt(Center(VideoClip(0,3)));
        TimelineKeyPress('e'); // Remove &empty space
        ASSERT_EQUALS(VideoTrack(1)->getLength(), len1 - gap);
        ASSERT_EQUALS(VideoTrack(0)->getLength(), len0 - gap);
        ASSERT_EQUALS(AudioTrack(0)->getLength(), len0 - gap);
        ASSERT_EQUALS(AudioTrack(1)->getLength(), len1 - gap);

        Undo(5);
    }
}

void TestUiTimelinePopupMenu::testOpenPopupMenuTwice()
{
    StartTestSuite();
    TimelineZoomIn(2);
    ConfigFixture.SnapToClips(false);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    // NOTE: Avoid the use of WaitForIdle while the popup menu is being shown.
    //       Popup menu blocks the idle events causing long delays...
    {
        StartTest("Open popup menu twice");
        TimelineOpenPopupMenuAt(Center(VideoClip(0,3)));
        ASSERT(getTimeline().getMenuHandler().isPopupShown());
        wxUIActionSimulator().MouseMove(TimelinePosition() + Center(VideoClip(0,3)) - wxPoint(150,0));
        wxUIActionSimulator().MouseClick(wxMOUSE_BTN_RIGHT);
        pause(2000);
        ASSERT(getTimeline().getMenuHandler().isPopupShown());
        TimelineClosePopupMenu();
        ASSERT(!getTimeline().getMenuHandler().isPopupShown());
    }
}

void TestUiTimelinePopupMenu::testRightClickScrollingAfterOpeningPopupMenu()
{
    StartTestSuite();
    TimelineZoomIn(4);
    ASSERT_EQUALS(getTimeline().getZoom().getCurrent(), rational(1,1));
    StartTest("Open popup menu then start a scrolling via right mouse down");
    ASSERT_MORE_THAN(getTimeline().GetVirtualSize().x,getTimeline().GetClientSize().x);
    TimelineOpenPopupMenuAt(Center(VideoClip(0,3)));
    ASSERT(getTimeline().getMenuHandler().isPopupShown());
    ASSERT_EQUALS(getTimeline().getScrolling().getOffset(),wxPoint(0,0));
    TimelineMoveLeft(150);
    TimelineRightDown();
    ASSERT(!getTimeline().getMenuHandler().isPopupShown());
    TimelineMoveLeft(150);
    ASSERT_MORE_THAN_EQUALS(getTimeline().getScrolling().getOffset().x, 150);
    SetWaitAfterEachInputAction(false);
    TimelineRightUp();
    SetWaitAfterEachInputAction(true);
    ASSERT(!getTimeline().getMenuHandler().isPopupShown());
}

void TestUiTimelinePopupMenu::testOpenPopupMenuWhenClickingOnTransition()
{
    StartTestSuite();
    TimelineZoomIn(6);

    pts lengthOfCrossFade =  Config::ReadLong(Config::sPathDefaultTransitionLength);
    pts lengthOfFade = lengthOfCrossFade / 2;

    {
        MakeInOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthOfCrossFade);

        TimelineMove(TransitionLeftClipInterior(VideoClip(0,2)));
        {
            StartTest("InOutTransition: TransitionLeftClipInterior: Fade in.");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT( wxEventLoopBase::GetActive()->IsMain());
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionLeftClipInterior: Fade out (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("InOutTransition: TransitionLeftClipInterior: Fade from previous.");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfCrossFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionLeftClipInterior: Fade to next (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("InOutTransition: TransitionLeftClipEnd: Fade in.");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionLeftClipEnd: Fade out (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("InOutTransition: TransitionLeftClipEnd: Fade from previous.");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfCrossFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionLeftClipEnd: Fade to next (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("InOutTransition: TransitionRightClipInterior: Fade in (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InOutTransition: TransitionRightClipInterior: Fade out.");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionRightClipInterior: Fade from previous (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InOutTransition: TransitionRightClipInterior: Fade to next.");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfCrossFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionRightClipBegin: Fade in (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InOutTransition: TransitionRightClipBegin: Fade out.");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("InOutTransition: TransitionRightClipBegin: Fade from previous (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InOutTransition: TransitionRightClipBegin: Fade to next.");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfCrossFade);
            Undo();
        }
    }
    {
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthOfFade);

        {
            StartTest("InTransition: TransitionRightClipInterior: Fade in (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InTransition: TransitionRightClipInterior: Fade out.");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("InTransition: TransitionRightClipInterior: Fade from previous (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InTransition: TransitionRightClipInterior: Fade to next.");
            TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfCrossFade);
            Undo();
        }
        {
            StartTest("InTransition: TransitionRightClipBegin: Fade in (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InTransition: TransitionRightClipBegin: Fade out.");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("InTransition: TransitionRightClipBegin: Fade from previous (ignored).");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        }
        {
            StartTest("InTransition: TransitionRightClipBegin: Fade to next.");
            TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfCrossFade);
            Undo();
        }
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthOfFade);

        {
            StartTest("OutTransition: TransitionLeftClipInterior: Fade in.");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT( wxEventLoopBase::GetActive()->IsMain());
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("OutTransition: TransitionLeftClipInterior: Fade out (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("OutTransition: TransitionLeftClipInterior: Fade from previous.");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfCrossFade);
            Undo();
        }
        {
            StartTest("OutTransition: TransitionLeftClipInterior: Fade to next (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("OutTransition: TransitionLeftClipEnd: Fade in.");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('i'); // Fade &in
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfFade);
            Undo();
        }
        {
            StartTest("OutTransition: TransitionLeftClipEnd: Fade out (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('o'); // Fade &out
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
        {
            StartTest("OutTransition: TransitionLeftClipEnd: Fade from previous.");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('p'); // Cross-fade from &previous
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfCrossFade);
            Undo();
        }
        {
            StartTest("OutTransition: TransitionLeftClipEnd: Fade to next (ignored).");
            TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
            TimelineKeyPress('n'); // Cross-fade to &next
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        }
    }
}

void TestUiTimelinePopupMenu::testUnlinkingAudioAndVideoClips()
{
    StartTestSuite();
    TimelineZoomIn(6);
    {
        StartTest("Unlink video from audio");
        ASSERT_EQUALS(VideoClip(0,1)->getLink(), AudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,1)->getLink(), VideoClip(0,1));
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('u'); // &unlink
        ASSERT_ZERO(VideoClip(0,1)->getLink());
        ASSERT_ZERO(AudioClip(0,1)->getLink());
        Undo();
        ASSERT_EQUALS(VideoClip(0,1)->getLink(), AudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,1)->getLink(), VideoClip(0,1));
    }
    {
        StartTest("Unlink audio from video");
        ASSERT_EQUALS(VideoClip(0,1)->getLink(), AudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,1)->getLink(), VideoClip(0,1));
        TimelineOpenPopupMenuAt(Center(AudioClip(0,1)));
        TimelineKeyPress('u'); // &unlink
        ASSERT_ZERO(VideoClip(0,1)->getLink());
        ASSERT_ZERO(AudioClip(0,1)->getLink());
        Undo();
        ASSERT_EQUALS(VideoClip(0,1)->getLink(), AudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,1)->getLink(), VideoClip(0,1));
    }
}

} // namespace