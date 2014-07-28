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

#include "TestTransition.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTransition::setUp()
{
    mProjectFixture.init();
}

void TestTransition::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTransition::testSelectionAndDeletion()
{
    StartTestSuite();
    Zoom level(4);

    {
        DeselectAllClips();
        MakeInOutTransitionAfterClip preparation(1);
        StartTest("InOutTransition: Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>(); // Was a bug once when clicking on a clip's begin/end
        StartTest("InOutTransition: Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>(); // Was a bug once when clicking on a clip's begin/end
        StartTest("InOutTransition: Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>(); // Was a bug once when clicking on a clip's begin/end
        StartTest("InOutTransition: Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>(); // Was a bug once when clicking on a clip's begin/end
    }
    {
        DeselectAllClips();
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("OutTransition: Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        StartTest("OutTransition: Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        StartTest("OutTransition: When deleting the associated clip, the transition must be deleted also.");
        DeselectAllClips();
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT_SELECTION_SIZE(1);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)            (VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        Undo();
    }
    {
        DeselectAllClips();
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("InTransition: Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        DeselectAllClips();
        StartTest("InTransition: Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        StartTest("InTransition: When deleting the associated clip, the transition must be deleted also.");
        DeselectAllClips();
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_SELECTION_SIZE(1);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)            (VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        Undo();
    }
    {
        DeselectAllClips();
        MakeInOutTransitionAfterClip preparation(1);
        StartTest("InOutTransition: Right Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        OpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ClosePopupMenu();
        StartTest("InOutTransition: Right Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        OpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ClosePopupMenu();
        StartTest("InOutTransition: Right Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        OpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ClosePopupMenu();
        StartTest("InOutTransition: Right Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        OpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ClosePopupMenu();
    }
    {
        DeselectAllClips();
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("OutTransition: Right Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        OpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ClosePopupMenu();
        StartTest("OutTransition: Right Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        OpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ClosePopupMenu();
    }
    {
        DeselectAllClips();
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("InTransition: Right Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        OpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ClosePopupMenu();
        StartTest("InTransition: Right Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        OpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ClosePopupMenu();
    }
}

void TestTransition::testMakeRoomForCrossfade()
{
    StartTestSuite();
    Zoom level(6);
    ConfigFixture.SnapToClips(false);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    {
        StartTest("Left clip - small hidden extension, right clip - none: Crossfade only has half the default length.");
        ShiftTrim(RightCenter(VideoClip(0,0)),RightCenter(VideoClip(0,0)) - wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 2),0)); // Make left clip 'extendable' on its right side, with length 12
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength / 2);
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength / 2);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Left clip - large hidden extension, right clip - none: Crossfade is positioned entirely to the right of the cut.");
        ShiftTrim(RightCenter(VideoClip(0,0)),Center(VideoClip(0,0))); // Make left clip 'extendable' on its right side
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Right clip - small 'hidden extension', left clip: none: Crossfade is positioned entirely to the left of the cut.");
        ShiftTrim(LeftCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) + wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 2),0)); // Make right clip 'extendable' on its left side, with length 12
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength / 2);
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip      )(      AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Right clip - large 'hidden extension', left clip: none: Crossfade is positioned entirely to the left of the cut.");
        ShiftTrim(LeftCenter(VideoClip(0,1)),Center(VideoClip(0,1))); // Make right clip 'extendable' on its left side
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip      )(      AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }

    {
        StartTest("Left clip - small 'hidden extension', right clip - large one: Crossfade positioned to the left of the cut mostly.");
        ShiftTrim(RightCenter(VideoClip(0,0)),RightCenter(VideoClip(0,0)) - wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 4),0)); // Make left clip 'extendable' on its right side, with length 6
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength / 4);
        ShiftTrim(LeftCenter(VideoClip(0,1)),Center(VideoClip(0,1))); // Make right clip 'extendable' (a lot) on its left side
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip - 3 * (defaultTransitionLength / 4) ); // 3/4 of the transition is 'to the left'
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip - defaultTransitionLength / 4); // 1/4 of the transition is 'to the right'
        Undo(3);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Left clip has large 'hidden extension', right a small one: Crossfade positioned as much to the right of the cut as possible.");
        ShiftTrim(RightCenter(VideoClip(0,0)),Center(VideoClip(0,0))); // Make left clip 'extendable' (a lot) on its right side
        ShiftTrim(LeftCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) + wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 4),0)); // Make right clip 'extendable' on its left side, with length 6
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength / 4);
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip - defaultTransitionLength / 4); // 1/4 of the transition is 'to the left'
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip - 3 * (defaultTransitionLength / 4) ); // 3/4 of the transition is 'to the right'
        Undo(3);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Left clip is not extendible, right is: Add crossfade to next clip."); // bug once
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), defaultTransitionLength / 2);
        ShiftTrim(RightCenter(VideoClip(0,1)), Center(VideoClip(0,0)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip); // No transition added
        Undo(2);
    }
    {
        StartTest("Left clip is not extendible, right is: Add crossfade from previous clip."); // bug once
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), defaultTransitionLength / 2);
        ShiftTrim(RightCenter(VideoClip(0,1)), Center(VideoClip(0,0)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        OpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip); // No transition added
        Undo(2);
    }
    {
        StartTest("Right clip is not extendible, left is: Add crossfade to next clip."); // bug once
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        ShiftTrim(LeftCenter(VideoClip(0,1)), Center(VideoClip(0,2)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Right clip is not extendible, left is: Add crossfade from previous clip."); // bug once
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        ShiftTrim(LeftCenter(VideoClip(0,1)), Center(VideoClip(0,2)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Both clips not extendible: Add crossfade to next clip."); // bug once
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        ShiftTrim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1)) + wxPoint(10,0));
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        ShiftTrim(RightCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) - wxPoint(10,0));
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(Transition);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), 0);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), 0);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), defaultTransitionLength / 2);
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(Transition)(VideoClip); // No transition added
        StartTest("Both clips not extendible: Add crossfade from previous clip."); // bug once
        OpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(Transition)(VideoClip); // No transition added
    }
}

void TestTransition::testDragAndDropOfOtherClips()
{
    StartTestSuite();
    Zoom level(3);
    MakeInOutTransitionAfterClip preparation(1);
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,2));
    {
        // Shift drag without snapping enabled,
        // transition and its adjacent clips are shifted backwards
        Drag(From(Center(VideoClip(0,6))).To(Center(VideoClip(0,3))).HoldShiftWhileDragging());
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        // Shift drag without snapping enabled (drop over clip AFTER
        // transition) clip after transition is shifted backwards ->
        // transition is removed because the two 'transitioned clips
        // are separated'. (clip in front of transition remains intact)
        Drag(From(Center(VideoClip(0,6))).To(Center(VideoClip(0,4))).HoldShiftWhileDragging());
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
    }
    ConfigFixture.SnapToClips(true);
    {
        // Shift drag with snapping enabled. The mouse drag is done such that the left position of the drop is aligned with the left
        // position of the clip left of the transitions. At that point, a difference of a couple of pixels can already change to which
        // cuts the snapping is done.
        pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
        StartTest("Snap almost to right edge of 00.avi");
        Drag(From(Center(VideoClip(0,6))).HoldShiftWhileDragging().AlignLeft(RightPixel(VideoClip(0,0))+26));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        Undo();
        StartTest("Snap to right edge of 00.avi");
        Drag(From(Center(VideoClip(0,6))).HoldShiftWhileDragging().AlignLeft(RightPixel(VideoClip(0,0))+25)); // Mouse must be moved a bit further to snap to the left edge
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that the left
        // position of the drop is aligned with the left position of
        // the transition. This causes the clip left of the transition
        // to be shifted back.
        pts lengthOfDraggedClip = VideoClip(0,5)->getLength();
        Drag(From(Center(VideoClip(0,5))).HoldShiftWhileDragging().AlignLeft(preparation.leftPositionOfTransitionAfterTransitionApplied));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,5)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that the left position@
        // of the drop is aligned with the center position of the transition.
        // This causes the clip right of the transition to be shifted back, and the transition
        // to be removed.
        pts lengthOfDraggedClip = VideoClip(0,5)->getLength();
        Drag(From(Center(VideoClip(0,5))).HoldShiftWhileDragging().AlignLeft(preparation.touchPositionOfTransition));
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that the left position of the drop is aligned
        // with the left position of the clip after the transition.
        // This causes the clip right of the transition to be shifted back, and the transition
        // to be removed.
        pts lengthOfDraggedClip = VideoClip(0,5)->getLength();
        Drag(From(Center(VideoClip(0,5))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,3))));
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
    }
    ConfigFixture.SnapToClips(false);
    {
        TrimRight(VideoClip(0,6),-100); // Make the dragged clip small enough for the subsequent two tests
        pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
        {
            // Drag a small clip on top of the clip left of the transition. This left clip is made shorter, but the transition remains.
            pixel right = RightPixel(VideoClip(0,1));
            right -= 20; // Ensure that 'a bit' of the clip left of the transition remains, causing the transition to remain also
            Drag(From(Center(VideoClip(0,6))).AlignRight(right));
            ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDraggedClip);
            ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
            ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfTransition);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
            Undo();
        }
        {
            // Drag a small clip on top of the clip right of the transition. This right clip is made shorter, but the transition remains.
            pixel left = LeftPixel(VideoClip(0,3));
            left += 20; // Ensure that 'a bit' of the clip right of the transition remains, causing the transition to remain also
            Drag(From(Center(VideoClip(0,6))).AlignLeft(left));
            ASSERT_EQUALS(VideoClip(0,4)->getLength(),lengthOfDraggedClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
            ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
            Undo();
        }
        Undo(); // TrimRight(VideoClip(0,6),-100)
    }
    {
        // Move the leftmost of the two clips adjacent to the transition: the transition must be removed
        DeselectAllClips();
        Drag(From(Center(VideoClip(0,1))).To(Center(VideoClip(0,4))));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
        ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
        Undo();
    }
    {
        // Move a large clip onto a smaller clip. This causes linking issues
        // (the video clip was not completely removed, but the linked audio
        // clip was - or vice versa? - anyway: crashed....)
        DeselectAllClips();
        TimelineLeftClick(Center(VideoClip(0,1)));
        Drag(From(LeftCenter(VideoClip(0,1)) + wxPoint(10,0)).To(Center(VideoClip(0,6))));
        Undo();
    }
    {
        // Drag and drop the clip onto (approx.) the same position. That scenario caused bugs:
        // clip is removed (during drag-and-drop). At the end of the drag-and-drop,
        // the transition is 'undone'. The undoing of the transition made assumptions
        // on availability of adjacent clips, which was invalid (clip has just been moved).
        wxPoint from = LeftCenter(VideoClip(0,1)) + wxPoint(10,0);
        wxPoint to = Center(VideoClip(0,6));
        Drag(From(from).To(to).DontReleaseMouse());
        Drag(From(to).To(from));
        Undo();
    }
}

void TestTransition::testDragAndDropOfClipsUnderTransition()
{
    StartTestSuite();
    Zoom level(4);
    {
        // Test - for an in-out-transition- that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
        MakeInOutTransitionAfterClip preparation(1);
        Drag(From(TransitionLeftClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied); // Although the clip is dropped onto position of clip 4, the unapply transition causes this to be number 3
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),AudioClip(0,3)->getLength()); // Transition is unapplied which causes the audio and video to have the same lengths again
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),AudioClip(0,4)->getLength()); // Transition is unapplied which causes the audio and video to have the same lengths again
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
    {
        // Test - for an in-out-transition - that dragging when clicking on TransitionRightClipInterior starts a
        // drag and drop operation, not with the transition but the clip right of the transition.
        MakeInOutTransitionAfterClip preparation(1);
        Drag(From(TransitionRightClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),AudioClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
    {
        // Test - for an in-only-transition - that dragging when clicking on TransitionRightClipInterior starts a
        // drag and drop operation, not with the transition but the clip right of the transition.
        // The transition in this case is dragged along with the clip, since the only clip related to the
        // transition is the clip being dragged.
        MakeInTransitionAfterClip preparation(1);
        Drag(From(TransitionRightClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength() + VideoClip(0,4)->getLength(),AudioClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
    {
        // Test - for an out-only-transition - that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
        // The transition in this case is dragged along with the clip, since the only clip related to the
        // transition is the clip being dragged.
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_EQUALS(VideoClip(0,1)->getLength() + VideoClip(0,2)->getLength(),AudioClip(0,1)->getLength()); // Transition is unapplied which causes the audio and video to have the same lengths again
        Drag(From(TransitionLeftClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied); // Clip and transition are replaced with emptyness
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength() + VideoClip(0,4)->getLength(),AudioClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
}

void TestTransition::testAdjacentTransitions()
{
    StartTestSuite();
    Zoom level(3);
    ConfigFixture.SnapToClips(true);
    {
        StartTest("Reduce size of second and third clip to be able to create transitions");
        TrimRight(VideoClip(0,1),-30,false);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        TrimLeft(VideoClip(0,3),30,false); // Note: the trim of clip 1 causes clip 2 to become clip 3 (clip 2 is empty space)
        ASSERT(!VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT(!VideoClip(0,4)->isA<model::EmptyClip>());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd())(VideoClip(0,1));
        ASSERT_LESS_THAN_ZERO(VideoClip(0,3)->getMinAdjustBegin())(VideoClip(0,2));
        // Make transitions between clips 2 and 3
        TimelineMove(RightCenter(VideoClip(0,1)));
        TimelineKeyPress('o');
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        TimelineMove(LeftCenter(VideoClip(0,4)));
        TimelineKeyPress('i');
        ASSERT(VideoClip(0,4)->isA<model::Transition>());
        ASSERT(!VideoClip(0,5)->isA<model::EmptyClip>());
        Drag(From(Center(VideoClip(0,5))).AlignLeft(RightPixel(VideoClip(0,2))));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        Scrub(LeftPixel(VideoTransition(0,2)) - 5, RightPixel(VideoTransition(0,3)) + 5);
        Play(LeftPixel(VideoTransition(0,2)) - 2, 500); // -2: Also take some frames from the left clip
    }
    {
        StartTest("Drag a clip just on top of the right transition.");
        // This effectively removes that right transition and (part) of the clip to its right.
        // The clips to the left of the removed transition must remain unaffected.
        pts cliplength = VideoClip(0,1)->getLength();
        pts transitionlength = VideoClip(0,2)->getLength();
        pts length = VideoClip(0,8)->getLength();
        Drag(From(Center(VideoClip(0,8))).AlignLeft(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),cliplength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),transitionlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),length);
        Undo();
    }
    {
        StartTest("Drag a clip just on top of the left transition.");
        // This effectively removes that left transition and (part) of the clip to its left.
        // The clips to the right of the removed transition must remain unaffected.
        pts cliplength = VideoClip(0,4)->getLength();
        pts transitionlength = VideoClip(0,3)->getLength();
        pts length = VideoClip(0,6)->getLength();
        Drag(From(Center(VideoClip(0,6)) + wxPoint(5,0)).AlignRight(RightPixel(VideoClip(0,2))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(EmptyClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),length);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),transitionlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),cliplength);
        Undo();
    }
}

void TestTransition::testPlaybackAndScrubbing()
{
    StartTestSuite();
    Zoom level(1); // Zoom in once to avoid clicking in the middle of a clip which is then seen (logically) as clip end due to the zooming
    {
        MakeInOutTransitionAfterClip preparation(1);
        StartTest("Select and delete InOutTransition only. Then, the remaining clips must have their original lengths restored.");
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        TimelineKeyPress(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();
        StartTest("Move clips around InOutTransition: the transition must be moved also.");
        DeselectAllClips();
        TimelineLeftClick(Center(VideoClip(0,1)));
        Drag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,5))).HoldCtrlBeforeDragStarts());
        //CtrlDrag(Center(VideoClip(0,3)), Center(VideoClip(0,5)));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(Transition);
        ASSERT(VideoTransition(0,5)->getRight());
        ASSERT_MORE_THAN_ZERO(*(VideoTransition(0,5)->getRight()));
        ASSERT(VideoTransition(0,5)->getLeft());
        ASSERT_MORE_THAN_ZERO(*(VideoTransition(0,5)->getLeft()));
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,9)->getRightPts(),AudioClip(0,8)->getRightPts());
        Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
        Play(LeftPixel(VideoTransition(0,5)) - 2, 500); // -2: Also take some frames from the left clip
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        Undo();
    }
    {
        MakeInTransitionAfterClip preparation(1);
        StartTest("Select and delete InTransition only. Then, the remaining clips must have their original lengths restored.");
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        TimelineKeyPress(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();
        StartTest("Move clip related to InTransition: the transition must be moved also.");
        DeselectAllClips();
        Drag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,5))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition);
        ASSERT(VideoTransition(0,5)->getRight());
        ASSERT_MORE_THAN_ZERO(*(VideoTransition(0,5)->getRight()));
        ASSERT(!VideoTransition(0,5)->getLeft());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());
        Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
        Play(LeftPixel(VideoTransition(0,5)) - 2, 500); // -2: Also take some frames from the left clip
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        Undo(); // Undo until the two trimmed clips are present
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        StartTest("Select and delete OutTransition only. Then, the remaining clips must have their original lengths restored.");
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        TimelineKeyPress(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();
        StartTest("Move clip related to OutTransition: the transition must be moved also.");
        DeselectAllClips();
        Drag(From(Center(VideoClip(0,1))).To(Center(VideoClip(0,5))));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(Transition);
        ASSERT(!VideoTransition(0,6)->getRight());
        ASSERT(VideoTransition(0,6)->getLeft());
        ASSERT_MORE_THAN_ZERO(*(VideoTransition(0,6)->getLeft()));
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());
        Scrub(LeftPixel(VideoTransition(0,6)) - 5, RightPixel(VideoTransition(0,6)) + 5);
        Play(LeftPixel(VideoTransition(0,6)) - 2, 500); // -1: Also take some frames from the left clip
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        Undo(); // Undo until the two trimmed clips are present
    }
}

void TestTransition::testTrimmingClipsInTransition()
{
    StartTestSuite();
    Zoom level(4);

    {
        MakeInOutTransitionAfterClip preparation(1);

        StartTest("InOutTransition: Without shift: TransitionLeftClipEnd: reduce clip size.");
        Trim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionLeftClipEnd: Verify lower resize bound (must be such that the entire clip can be trimmed away, since the transition is unapplied).");
        Trim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionLeftClipEnd: enlarge clip size (is not possible).");
        Trim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: reduce the clip size.");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: Verify lower resize bound (must be such that of the left clip only the part under the transition remains).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: enlarge clip size (which is possible when shift dragging).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,2)));
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: Verify upper resize bound (only useful for shift dragging).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionOriginal - preparation.lengthOfTransition);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        StartTest("OutTransition: Without shift: TransitionLeftClipEnd: reducing clip size");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        Trim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip)(VideoClip);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("OutTransition: Without shift: TransitionLeftClipEnd: Verify lower resize bound (the entire clip can be trimmed such that only the part 'under' the transition remains).");
        Trim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip)(VideoClip);
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        StartTest("OutTransition: Without shift: TransitionLeftClipEnd: enlarge clip size (is not possible).");
        Trim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("OutTransition: Without shift: ClipEnd: reduce clip size (verify that transition keeps being positioned alongside the clip.)");
        TrimRight(VideoClip(0,2),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip)(VideoClip);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,1)->getLength() + VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfTransition);
        StartTest("OutTransition: Without shift: ClipEnd: enlarge clip size (verify that transition keeps being positioned alongside the clip.)");
        TrimRight(VideoClip(0,2),20,false);
        if (!VideoClip(0,3)->isA<model::VideoClip>())
        {
            DumpSequenceAndWait();
        }
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfTransition);
        Undo();
        Undo();
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: reduce clip size (done without undo, since the 'reduce' makes room for enlarging).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd());
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: enlarge the clip size (which is possible since the left clip was just shortened).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo(); // Undo Trim 2
        Undo(); // Undo Trim 1
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: Verify lower resize bound (which is such that of the left clip only the part under the transition remains).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: enlarge clip size directly after creating the transition (impossible since left clip cannot be extended further).");
        ShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        // NOT: Undo();
    }
    {
        MakeInOutTransitionAfterClip preparation(1);

        StartTest("InOutTransition: Without shift: TransitionRightClipBegin: reduce clip size.");
        Trim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionRightClipBegin: Verify upper resize bound (which must be such that the entire clip can be trimmed away, since the transition is unapplied).");
        Trim(TransitionRightClipBegin(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionRightClipBegin: enlarge clip size (which is not possible).");
        Trim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: reduce clip size.");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: Verify upper resize bound (which is such that of the right clip only the part under the transition remains).");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT(!VideoClip(0,3)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        Undo();
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: enlarge clip size (which is possible when shift dragging).");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,2)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: Verify lower resize bound (only useful for shift dragging).");
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionOriginal - preparation.lengthOfTransition);
        Undo();
    }
    {
        MakeInTransitionAfterClip preparation(1);

        StartTest("InTransition: Without shift: TransitionRightClipBegin: reduce clip size.");
        Trim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfTransition);
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InTransition: Without shift: TransitionRightClipBegin: Verify upper resize bound (the entire clip can be trimmed such that only the part 'under' the transition remains).");
        Trim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfTransition);
        ASSERT_ZERO(VideoClip(0,4)->getLength());
        Undo();
        StartTest("InTransition: Without shift: TransitionRightClipBegin: enlarge clip size (which is not possible).");
        Trim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("InTransition: Without shift: ClipEnd: reduce clip size (verify that transition keeps being positioned alongside the clip.)");
        TrimLeft(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,2)->getLength());
        ASSERT_EQUALS(VideoClip(0,2)->getLength() + VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), preparation.lengthOfTransition);
        StartTest("InTransition: Without shift: ClipEnd: enlarge clip size (verify that transition keeps being positioned alongside the clip.)");
        TrimLeft(VideoClip(0,3),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfTransition);
        Undo();
        Undo();
        StartTest("InTransition: With shift: TransitionRightClipBegin: reduce clip size (done without undo, since the 'reduce' makes room for enlarging).");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,3)->getMinAdjustBegin());
        StartTest("InTransition: With shift: TransitionRightClipBegin: enlarge the clip size (which is possible since the left clip was just shortened).");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo(); // Undo Trim 2
        Undo(); // Undo Trim 1
        StartTest("InTransition: With shift: TransitionRightClipBegin: Verify upper resize bound (which is such that of the right clip only the part under the transition remains).");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),0); // The right clip itselves has length 0, only the part under the transition is used
        Undo();
        StartTest("InTransition: With shift: TransitionRightClipBegin: enlarge clip size directly after creating the transition (impossible since right clip cannot be extended further).");
        ShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        // NOT: Undo();
    }
}

void TestTransition::testTrimmingLinkedClips()
{
    StartTestSuite();
    Zoom Level(4);
    {
        MakeInOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();
        StartTest("InOutTransition: Without shift: reduce size of clip linked to the out-clip (transition must be removed)");
        Trim(RightCenter(AudioClip(0,1)),Center(AudioClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the size of the clip linked to the in-clip (transition must be removed)");
        Trim(LeftCenter(AudioClip(0,2)),Center(AudioClip(0,2)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the size of the clip linked to the out-clip  as much as possible (transition must be removed)");
        Trim(RightCenter(AudioClip(0,1)),LeftCenter(AudioClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the size of the clip linked to the in-clip as much as possible (transition between videos must be removed)");
        pts lengthOfVideoClipAfterOutClip = VideoClip(0,4)->getLength();
        pts lengthOfAudioClipLinkedToVideoClipAfterOutClip = AudioClip(0,3)->getLength();
        Trim(LeftCenter(AudioClip(0,2)),RightCenter(AudioClip(0,3)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),lengthOfVideoClipAfterOutClip);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),lengthOfAudioClipLinkedToVideoClipAfterOutClip);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the 'other' side of the clip linked to the in-clip as much as possible (transition is NOT removed)");
        Trim(LeftCenter(AudioClip(0,1)),RightCenter(AudioClip(0,2)));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(AudioClip )(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_ZERO(VideoClip(0,2)->getLength());
        ASSERT_EQUALS(AudioClip(0,1)->getLength() + AudioClip(0,2)->getLength(), originalLengthOfAudioClip1);
        StartTest("InOutTransition: Without shift: scrub with an 'in' clip which is fully obscured by the transition");
        Scrub(RightPixel(VideoClip(0,1))-5, HCenter(VideoClip(0,3))); // Bug once: The empty video clip that is 'just before' the transition asserted when doing a moveTo(0), since !(0<length) for a clip with length 0.
        StartTest("InOutTransition: Without shift: enlarge 'in' clip which is fully obscured by the transition");
        Trim(LeftVBottomQuarter(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        Undo();
        Undo();
        StartTest("InOutTransition: Without shift: reduce 'other' side of the clip linked to the out-clip as much as possible (transition is NOT removed)");
        Trim(RightCenter(AudioClip(0,2)),Center(AudioClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_ZERO(VideoClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,2)->getLength() + AudioClip(0,3)->getLength(), originalLengthOfAudioClip2);
        StartTest("InOutTransition: Without shift: scrub an 'out' clip which is fully obscured by the transition");
        Scrub(HCenter(VideoClip(0,2)), LeftPixel(VideoClip(0,4)) + 5); // Bug once: The empty video clip that is 'just before' the transition asserted when doing a moveTo(0), since !(0<length) for a clip with length 0.
        StartTest("InOutTransition: Without shift: enlarge 'out' clip which is fully obscured by the transition");
        Trim(RightVBottomQuarter(VideoClip(0,3)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),originalLengthOfAudioClip2);
        Undo();
        Undo();
    }
}

void TestTransition::testTrimmingTransition()
{
    StartTestSuite();
    Zoom Level(4);
    ConfigFixture.SnapToClips(false).SnapToCursor(false);
    {
        MakeInOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();
        StartTest("InOutTransition: Trim left: Reduce size.");
        wxPoint aBitToTheRight = VTopQuarterLeft(VideoClip(0,2)) + wxPoint(20,0);
        Trim(VTopQuarterLeft(VideoClip(0,2)),aBitToTheRight);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Trim left: Reduce size and verify upper resize bound.");
        Trim(VTopQuarterLeft(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoTransition(0,2)->getLeft());
        ASSERT_ZERO(*(VideoTransition(0,2)->getLeft()));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: Trim left: Enlarge size and verify lower resize bound (bound imposed by the available extra data in the clip right of the transition).");
        Trim(VTopQuarterLeft(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getLength());
        ASSERT_ZERO(VideoClip(0,2)->getMinAdjustBegin());
        ASSERT_MORE_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Trim left: Enlarge size and verify lower resize bound (bound imposed by the length of the clip left of the transition).");
        Trim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1)) + wxPoint(-10,0));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)            (AudioClip)(AudioClip);
        Trim(VTopQuarterLeft(VideoClip(0,3)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_ZERO(VideoClip(0,2)->getLength());
        ASSERT_ZERO(VideoClip(0,3)->getMinAdjustBegin());
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        Undo();
        StartTest("InOutTransition: Trim right: Reduce size.");
        wxPoint aBitToTheLeft = VTopQuarterRight(VideoClip(0,2)) - wxPoint(20,0);
        Trim(VTopQuarterRight(VideoClip(0,2)),aBitToTheLeft);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Trim right: Reduce size and verify lower resize bound.");
        Trim(VTopQuarterRight(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoTransition(0,2)->getRight());
        ASSERT_ZERO(*(VideoTransition(0,2)->getRight()));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Trim right: Enlarge size and verify upper resize bound (bound imposed by the available extra data in the clip left of the transition).");
        Trim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_ZERO(VideoClip(0,2)->getMaxAdjustEnd());
        ASSERT_MORE_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,3)->getLength());
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Trim right: Enlarge size and verify upper resize bound (bound imposed by the length of the clip right of the transition).");
        Trim(RightCenter(VideoClip(0,3)), LeftCenter(VideoClip(0,3)) + wxPoint(10,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(EmptyClip)(AudioClip);
        Trim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_ZERO(VideoClip(0,2)->getMaxAdjustEnd());
        ASSERT_ZERO(VideoClip(0,3)->getLength());
        ASSERT_MORE_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        Undo();
        Undo();
    }
    {
        MakeInTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();
        StartTest("InTransition: Trim left: Reduce size (impossible).");
        wxPoint aBitToTheRight = VTopQuarterLeft(VideoClip(0,2)) + wxPoint(20,0);
        Trim(VTopQuarterLeft(VideoClip(0,2)),aBitToTheRight);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        StartTest("InTransition: Trim left: Enlarge size (impossible).");
        Trim(VTopQuarterLeft(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        StartTest("InTransition: Trim right: Reduce size.");
        wxPoint aBitToTheLeft = VTopQuarterRight(VideoClip(0,2)) - wxPoint(5,0);
        Trim(VTopQuarterRight(VideoClip(0,2)),aBitToTheLeft);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InTransition: Trim right: Reduce size and verify lower resize bound.");
        Trim(VTopQuarterRight(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InTransition: Trim right: Enlarge size and verify upper resize bound (bound imposed by the length of the clip right of the transition).");
        Trim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_ZERO(VideoClip(0,2)->getMaxAdjustEnd());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_ZERO(VideoClip(0,3)->getLength());
        Undo();
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();
        StartTest("OutTransition: Trim left: Reduce size.");
        wxPoint aBitToTheRight = VTopQuarterLeft(VideoClip(0,2)) + wxPoint(5,0);
        Trim(VTopQuarterLeft(VideoClip(0,2)),aBitToTheRight);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("OutTransition: Trim left: Reduce size and verify upper resize bound.");
        Trim(VTopQuarterLeft(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("OutTransition: Trim left: Enlarge size and verify lower resize bound (bound imposed by the length of the clip left of the transition).");
        Trim(VTopQuarterLeft(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_ZERO(VideoClip(0,2)->getMinAdjustBegin());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("OutTransition: Trim right: Reduce size (impossible).");
        wxPoint aBitToTheLeft = VTopQuarterRight(VideoClip(0,2)) - wxPoint(20,0);
        Trim(VTopQuarterRight(VideoClip(0,2)),aBitToTheLeft);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("OutTransition: Trim right: Enlarge size (impossible).");
        Trim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
    }
    // Found bugs during development (once a bug with zoom level 3 and once with level 5)
    {
        StartTest("InTransition: Try enlarging the transition on the left side (known crash in ClipView for finding the correct position)");
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        DeleteClip(VideoClip(0,1));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(Transition)(VideoClip);
        TrimLeft(VideoClip(0,2),20);
        Trim(VTopQuarterLeft(VideoClip(0,2)),Center(VideoClip(0,1)));
        Undo(2);
    }
    {
        StartTest("OutTransition: Try enlarging the transition on the right side (known crash in ClipView for finding the correct position)");
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        DeleteClip(VideoClip(0,3));
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip);
        TrimRight(VideoClip(0,2),-20);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
        Trim(VTopQuarterRight(VideoClip(0,2)),Center(VideoClip(0,4)));
        Undo();
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();
    }
    {
        StartTest("InTransition: Try enlarging the transition on the left side (known crash in Transition::getMinAdjustBegin)");
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        DeleteClip(VideoClip(0,1));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(Transition)(VideoClip);
        TrimLeft(VideoClip(0,2),20);
        Trim(VTopQuarterLeft(VideoClip(0,2)),Center(VideoClip(0,1)));
        Undo(2);
    }
    {
        StartTest("OutTransition: Try enlarging the transition on the right side (known crash in Transition::getMaxAdjustEnd)");
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        DeleteClip(VideoClip(0,3));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip);
        TrimRight(VideoClip(0,2),-20);
        Trim(VTopQuarterRight(VideoClip(0,2)),Center(VideoClip(0,4)));
        Undo(2);
    }
}

void TestTransition::testCompletelyTrimmingAwayTransition()
{
    StartTestSuite();
    Zoom Level(4);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    {
        MakeInOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        pts lengthleft = VideoClip(0,1)->getLength();
        pts lengthright = VideoClip(0,3)->getLength();
        wxPoint from = VTopQuarterLeft(VideoClip(0,2));
        Trim(from,from + wxPoint(100,0));
        from = VTopQuarterRight(VideoClip(0,2));
        Trim(from,from + wxPoint(-100,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthleft + defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthright + defaultTransitionLength / 2);
        Undo(2);
    }
    {
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        pts lengthleft = VideoClip(0,1)->getLength();
        pts lengthright = VideoClip(0,3)->getLength();
        wxPoint from = VTopQuarterRight(VideoClip(0,2));
        Trim(from,from + wxPoint(-100,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthleft);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthright + defaultTransitionLength / 2);
        Undo();
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        pts lengthleft = VideoClip(0,1)->getLength();
        pts lengthright = VideoClip(0,3)->getLength();
        wxPoint from = VTopQuarterLeft(VideoClip(0,2));
        Trim(from,from + wxPoint(100,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthleft + defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthright);
        Undo();
    }
}

void TestTransition::testSplitNearZeroLengthEdgeOfTransition()
{
    StartTestSuite();
    Zoom level(5);
    MakeInOutTransitionAfterClip preparation(1);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    {
        StartTest("Left size is 0");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);

        // Reduce left part of transition to 0
        wxPoint from = VTopQuarterLeft(VideoClip(0,2));
        Trim(from,from + wxPoint(100,0));
        
        pts length1 = VideoClip(0,1)->getLength();
        pts length2 = VideoClip(0,3)->getLength();
        
        PositionCursor(10); // Was required to get the next position properly
        PositionCursor(LeftPixel(VideoClip(0,2))); // Ensure that the split is done exactly at the left edge of the transition
        TimelineKeyPress('s');
        
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip); // Transition removed
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), length1);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), length2 + defaultTransitionLength / 2); // The clip part under the transition is 'added to the adjacent clip'
        Undo(2);
    }
    {
        StartTest("Right size is 0");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        
        // Reduce right part of transition to 0
        wxPoint from = VTopQuarterRight(VideoClip(0,2));
        Trim(from,from + wxPoint(-100,0));

        pts length1 = VideoClip(0,1)->getLength();
        pts length2 = VideoClip(0,3)->getLength();
        
        PositionCursor(10); // Was required to get the next position properly
        PositionCursor(LeftPixel(VideoClip(0,3))); // Ensure that the split is done exactly at the right edge of the transition
        TimelineKeyPress('s');
        
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip); // Transition removed
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), length1 + defaultTransitionLength / 2); // The clip part under the transition is 'added to the adjacent clip'
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), length2);
        Undo(2);
    }
}


void TestTransition::testAudioTransitions()
{
    StartTestSuite();
    Zoom level(3);
    {
        MakeInOutTransitionAfterClip preparation(1,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
    {
        MakeInTransitionAfterClip preparation(1,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
    {
        MakeOutTransitionAfterClip preparation(1,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
    {
        MakeInOutTransitionAfterClip preparationAudio(1,true);
        MakeInOutTransitionAfterClip preparationVideo(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
        Scrub(LeftPixel(AudioClip(0,2)) - 5,RightPixel(AudioClip(0,2)) + 5);
        Play(HCenter(AudioClip(0,2)), 250);
    }
}

void TestTransition::testCreateTransitionAfterLastClip()
{
    StartTestSuite();
    Zoom level(1); // Zoom in once to avoid clicking in the middle of a clip which is then seen (logically) as clip end due to the zooming
    TrimRight(VideoClip(0,2), 20);
    {
        StartTest("Create transition after last video clip in track (NOTE: clip is NOT followed by EmptyClip).");
        Drag(From(Center(VideoClip(0,2))).To(wxPoint(RightPixel(VideoTrack(0)), VCenter(VideoTrack(0)))));
        TimelineMove(wxPoint(RightPixel(VideoTrack(0)), VCenter(VideoTrack(0))));
        TimelineKeyPress('o');
        ASSERT(VideoClip(0,8)->isA<model::Transition>());
        Undo(2);
    }
    {
        StartTest("Create transition after last audio clip in track (NOTE: clip is NOT followed by EmptyClip).");
        Drag(From(Center(AudioClip(0,2))).To(wxPoint(RightPixel(AudioTrack(0)), VCenter(AudioTrack(0)))));
        TimelineMove(wxPoint(RightPixel(AudioTrack(0)), VCenter(AudioTrack(0))));
        TimelineKeyPress('o');
        ASSERT(AudioClip(0,8)->isA<model::Transition>());
        Undo(2);
    }
}

void TestTransition::testVideoTransitionTypes()
{
    auto setOpacity = [](model::IClipPtr clip, int opacity)
    {
        model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(clip);
        ASSERT(videoclip);
        model::ChangeVideoClipTransform* cmd = new model::ChangeVideoClipTransform(videoclip);
        cmd->setOpacity(opacity);
        RunInMainAndWait([cmd]{ model::ProjectModification::submitIfPossible(cmd); });
    };

    StartTestSuite();
    Zoom level(6);
    for ( model::TransitionDescription t : model::video::VideoTransitionFactory::get().getAllPossibleTransitions() )
    {
        {
            StartTest("FadeIn: " + t.first);
            RunInMainAndWait([t]() { gui::timeline::command::createTransition(getSequence(), VideoClip(0,1),model::TransitionTypeIn, model::video::VideoTransitionFactory::get().getTransition(t)); });
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip);
            Scrub(-2 + LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)) + 2);
            StartTest("FadeIn with opacity: " + t.first);
            setOpacity(VideoClip(0,2),128);
            Scrub(-2 + LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)) + 2);
            Undo(2);
        }
        {
            StartTest("FadeOut: " + t.first);
            RunInMainAndWait([t]() { gui::timeline::command::createTransition(getSequence(), VideoClip(0,1),model::TransitionTypeOut, model::video::VideoTransitionFactory::get().getTransition(t)); });
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            Scrub(-2 + LeftPixel(VideoClip(0,2)), RightPixel(VideoClip(0,2)) + 2);
            StartTest("FadeOut with opacity: " + t.first);
            setOpacity(VideoClip(0,1),128);
            Scrub(-2 + LeftPixel(VideoClip(0,2)), RightPixel(VideoClip(0,2)) + 2);
            Undo(2);
        }
        {
            StartTest("FadeInOut: " + t.first);
            RunInMainAndWait([t]() { gui::timeline::command::createTransition(getSequence(), VideoClip(0,1),model::TransitionTypeInOut, model::video::VideoTransitionFactory::get().getTransition(t)); });
            ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip);
            Scrub(-2 + LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)) + 2);
            StartTest("FadeInOut with opacity: " + t.first);
            setOpacity(VideoClip(0,0),50);
            setOpacity(VideoClip(0,2),200);
            Scrub(-2 + LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)) + 2);
            Undo(3);
        }
        {
            StartTest("FadeOutIn: " + t.first);
            RunInMainAndWait([t]() { gui::timeline::command::createTransition(getSequence(), VideoClip(0,1),model::TransitionTypeOutIn, model::video::VideoTransitionFactory::get().getTransition(t)); });
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
            Scrub(-2 + LeftPixel(VideoClip(0,2)), RightPixel(VideoClip(0,2)) + 2);
            StartTest("FadeOutIn with opacity: " + t.first);
            setOpacity(VideoClip(0,1),128);
            setOpacity(VideoClip(0,3),222);
            Scrub(-2 + LeftPixel(VideoClip(0,2)), RightPixel(VideoClip(0,2)) + 2);
            Undo(3);
        }
    }
}

} // namespace