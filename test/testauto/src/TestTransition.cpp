// Copyright 2013-2016 Eric Raijmakers.
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
    TimelineZoomIn(4);

    {
        TimelineSelectClips({});
        MakeInOutTransitionAfterClip preparation(1);
        StartTest("InOutTransition: Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Was a bug once when clicking on a clip's begin/end
        StartTest("InOutTransition: Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Was a bug once when clicking on a clip's begin/end
        StartTest("InOutTransition: Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Was a bug once when clicking on a clip's begin/end
        StartTest("InOutTransition: Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Was a bug once when clicking on a clip's begin/end
    }
    {
        TimelineSelectClips({});
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("OutTransition: Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        StartTest("OutTransition: Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        TimelineLeftClick(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        StartTest("OutTransition: When deleting the associated clip, the transition must be deleted also.");
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT_SELECTION_SIZE(1);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)            (VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        Undo();
    }
    {
        TimelineSelectClips({});
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("InTransition: Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        StartTest("InTransition: Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        TimelineLeftClick(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        StartTest("InTransition: When deleting the associated clip, the transition must be deleted also.");
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_SELECTION_SIZE(1);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)            (VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        Undo();
    }
}

void TestTransition::testDragAndDropOfOtherClips()
{
    StartTestSuite();
    TimelineZoomIn(3);
    MakeInOutTransitionAfterClip preparation(1);
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,2));
    {
        // Shift drag without snapping enabled,
        // transition and its adjacent clips are shifted backwards
        TimelineDrag(From(Center(VideoClip(0,6))).To(Center(VideoClip(0,3))).HoldShiftWhileDragging());
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
        TimelineDrag(From(Center(VideoClip(0,6))).To(Center(VideoClip(0,4))).HoldShiftWhileDragging());
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
        TimelineDrag(From(Center(VideoClip(0,6))).HoldShiftWhileDragging().AlignLeft(RightPixel(VideoClip(0,0))+26));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        Undo();
        StartTest("Snap to right edge of 00.avi");
        TimelineDrag(From(Center(VideoClip(0,6))).HoldShiftWhileDragging().AlignLeft(RightPixel(VideoClip(0,0))+25)); // Mouse must be moved a bit further to snap to the left edge
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
        TimelineDrag(From(Center(VideoClip(0,5))).HoldShiftWhileDragging().AlignLeft(preparation.leftPositionOfTransitionAfterTransitionApplied));
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
        TimelineDrag(From(Center(VideoClip(0,5))).HoldShiftWhileDragging().AlignLeft(preparation.touchPositionOfTransition));
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
        TimelineDrag(From(Center(VideoClip(0,5))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,3))));
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
        TimelineTrimRight(VideoClip(0,6),-100); // Make the dragged clip small enough for the subsequent two tests
        pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
        {
            // Drag a small clip on top of the clip left of the transition. This left clip is made shorter, but the transition remains.
            pixel right = RightPixel(VideoClip(0,1));
            right -= 20; // Ensure that 'a bit' of the clip left of the transition remains, causing the transition to remain also
            TimelineDrag(From(Center(VideoClip(0,6))).AlignRight(right));
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
            TimelineDrag(From(Center(VideoClip(0,6))).AlignLeft(left));
            ASSERT_EQUALS(VideoClip(0,4)->getLength(),lengthOfDraggedClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
            ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
            Undo();
        }
        Undo(); // TimelineTrimRight(VideoClip(0,6),-100)
    }
    {
        // Move the leftmost of the two clips adjacent to the transition: the transition must be removed
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,1))).To(Center(VideoClip(0,4))));
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
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineDrag(From(LeftCenter(VideoClip(0,1)) + wxPoint(10,0)).To(Center(VideoClip(0,6))));
        Undo();
    }
    {
        // Drag and drop the clip onto (approx.) the same position. That scenario caused bugs:
        // clip is removed (during drag-and-drop). At the end of the drag-and-drop,
        // the transition is 'undone'. The undoing of the transition made assumptions
        // on availability of adjacent clips, which was invalid (clip has just been moved).
        wxPoint from = LeftCenter(VideoClip(0,1)) + wxPoint(10,0);
        wxPoint to = Center(VideoClip(0,6));
        TimelineDrag(From(from).To(to).DontReleaseMouse());
        TimelineDrag(From(to).To(from));
        Undo();
    }
}

void TestTransition::testDragAndDropOfClipsUnderTransition()
{
    StartTestSuite();
    TimelineZoomIn(4);
    {
        // Test - for an in-out-transition- that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
        MakeInOutTransitionAfterClip preparation(1);
        TimelineDrag(From(TransitionLeftClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
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
        TimelineDrag(From(TransitionRightClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
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
        TimelineDrag(From(TransitionRightClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
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
        TimelineDrag(From(TransitionLeftClipInterior(VideoClip(0,2))).HoldShiftWhileDragging().AlignLeft(LeftPixel(VideoClip(0,4))));
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
    TimelineZoomIn(3);
    ConfigFixture.SnapToClips(true);
    {
        StartTest("Reduce size of second and third clip to be able to create transitions");
        TimelineTrimRight(VideoClip(0,1),-30,false);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        TimelineTrimLeft(VideoClip(0,3),30,false); // Note: the trim of clip 1 causes clip 2 to become clip 3 (clip 2 is empty space)
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
        TimelineDrag(From(Center(VideoClip(0,5))).AlignLeft(RightPixel(VideoClip(0,2))));
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
        TimelineDrag(From(Center(VideoClip(0,8))).AlignLeft(LeftPixel(VideoClip(0,3))));
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
        TimelineDrag(From(Center(VideoClip(0,6)) + wxPoint(5,0)).AlignRight(RightPixel(VideoClip(0,2))));
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
    TimelineZoomIn(1); // Zoom in once to avoid clicking in the middle of a clip which is then seen (logically) as clip end due to the zooming
    {
        MakeInOutTransitionAfterClip preparation(1);
        StartTest("Select and delete InOutTransition only. Then, the remaining clips must have their original lengths restored.");
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        TimelineKeyPress(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_HISTORY_END(gui::timeline::cmd::DeleteSelectedClips);
        Undo();
        StartTest("Move clips around InOutTransition: the transition must be moved also.");
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineDrag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,5))).HoldCtrlBeforeDragStarts());
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
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
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
        ASSERT_HISTORY_END(gui::timeline::cmd::DeleteSelectedClips);
        Undo();
        StartTest("Move clip related to InTransition: the transition must be moved also.");
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,5))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(Transition);
        ASSERT(VideoTransition(0,5)->getRight());
        ASSERT_MORE_THAN_ZERO(*(VideoTransition(0,5)->getRight()));
        ASSERT(!VideoTransition(0,5)->getLeft());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());
        Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
        Play(LeftPixel(VideoTransition(0,5)) - 2, 500); // -2: Also take some frames from the left clip
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
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
        ASSERT_HISTORY_END(gui::timeline::cmd::DeleteSelectedClips);
        Undo();
        StartTest("Move clip related to OutTransition: the transition must be moved also.");
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,1))).To(Center(VideoClip(0,5))));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(Transition);
        ASSERT(!VideoTransition(0,6)->getRight());
        ASSERT(VideoTransition(0,6)->getLeft());
        ASSERT_MORE_THAN_ZERO(*(VideoTransition(0,6)->getLeft()));
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());
        Scrub(LeftPixel(VideoTransition(0,6)) - 5, RightPixel(VideoTransition(0,6)) + 5);
        Play(LeftPixel(VideoTransition(0,6)) - 2, 500); // -1: Also take some frames from the left clip
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
        Undo(); // Undo until the two trimmed clips are present
    }
}

void TestTransition::testTrimmingClipsInTransition()
{
    StartTestSuite();
    TimelineZoomIn(4);

    {
        MakeInOutTransitionAfterClip preparation(1);

        StartTest("InOutTransition: Without shift: TransitionLeftClipEnd: reduce clip size.");
        TimelineTrim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionLeftClipEnd: Verify lower resize bound (must be such that the entire clip can be trimmed away, since the transition is unapplied).");
        TimelineTrim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionLeftClipEnd: enlarge clip size (is not possible).");
        TimelineTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: reduce the clip size.");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: Verify lower resize bound (must be such that of the left clip only the part under the transition remains).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: enlarge clip size (which is possible when shift dragging).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,2)));
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionLeftClipEnd: Verify upper resize bound (only useful for shift dragging).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionOriginal - preparation.lengthOfTransition);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        StartTest("OutTransition: Without shift: TransitionLeftClipEnd: reducing clip size");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineTrim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip)(VideoClip);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("OutTransition: Without shift: TransitionLeftClipEnd: Verify lower resize bound (the entire clip can be trimmed such that only the part 'under' the transition remains).");
        TimelineTrim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip)(VideoClip);
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        StartTest("OutTransition: Without shift: TransitionLeftClipEnd: enlarge clip size (is not possible).");
        TimelineTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("OutTransition: Without shift: ClipEnd: reduce clip size (verify that transition keeps being positioned alongside the clip.)");
        TimelineTrimTransitionLeftClipEnd(VideoClip(0,2),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip)(VideoClip);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,1)->getLength() + VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfTransition);
        StartTest("OutTransition: Without shift: ClipEnd: enlarge clip size (verify that transition keeps being positioned alongside the clip.)");
        TimelineTrimTransitionLeftClipEnd(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfTransition);
        Undo();
        Undo();
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: reduce clip size (done without undo, since the 'reduce' makes room for enlarging).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd());
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: enlarge the clip size (which is possible since the left clip was just shortened).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo(); // Undo Trim 2
        Undo(); // Undo Trim 1
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: Verify lower resize bound (which is such that of the left clip only the part under the transition remains).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("OutTransition: With shift: TransitionLeftClipEnd: enlarge clip size directly after creating the transition (impossible since left clip cannot be extended further).");
        TimelineShiftTrim(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        // NOT: Undo();
    }
    {
        MakeInOutTransitionAfterClip preparation(1);

        StartTest("InOutTransition: Without shift: TransitionRightClipBegin: reduce clip size.");
        TimelineTrim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionRightClipBegin: Verify upper resize bound (which must be such that the entire clip can be trimmed away, since the transition is unapplied).");
        TimelineTrim(TransitionRightClipBegin(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Without shift: TransitionRightClipBegin: enlarge clip size (which is not possible).");
        TimelineTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: reduce clip size.");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: Verify upper resize bound (which is such that of the right clip only the part under the transition remains).");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT(!VideoClip(0,3)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        Undo();
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: enlarge clip size (which is possible when shift dragging).");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,2)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: With shift: TransitionRightClipBegin: Verify lower resize bound (only useful for shift dragging).");
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionOriginal - preparation.lengthOfTransition);
        Undo();
    }
    {
        MakeInTransitionAfterClip preparation(1);

        StartTest("InTransition: Without shift: TransitionRightClipBegin: reduce clip size.");
        TimelineTrim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfTransition);
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InTransition: Without shift: TransitionRightClipBegin: Verify upper resize bound (the entire clip can be trimmed such that only the part 'under' the transition remains).");
        TimelineTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfTransition);
        ASSERT_ZERO(VideoClip(0,4)->getLength());
        Undo();
        StartTest("InTransition: Without shift: TransitionRightClipBegin: enlarge clip size (which is not possible).");
        TimelineTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("InTransition: Without shift: ClipEnd: reduce clip size (verify that transition keeps being positioned alongside the clip.)");
        TimelineTrimTransitionRightClipBegin(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(Transition)(VideoClip);
        ASSERT_MORE_THAN_ZERO(VideoClip(0,2)->getLength());
        ASSERT_EQUALS(VideoClip(0,2)->getLength() + VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), preparation.lengthOfTransition);
        StartTest("InTransition: Without shift: ClipEnd: enlarge clip size (verify that transition keeps being positioned alongside the clip.)");
        TimelineTrimTransitionRightClipBegin(VideoClip(0,3),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfTransition);
        Undo();
        Undo();
        StartTest("InTransition: With shift: TransitionRightClipBegin: reduce clip size (done without undo, since the 'reduce' makes room for enlarging).");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),Center(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,3)->getMinAdjustBegin());
        StartTest("InTransition: With shift: TransitionRightClipBegin: enlarge the clip size (which is possible since the left clip was just shortened).");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo(); // Undo Trim 2
        Undo(); // Undo Trim 1
        StartTest("InTransition: With shift: TransitionRightClipBegin: Verify upper resize bound (which is such that of the right clip only the part under the transition remains).");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),0); // The right clip itselves has length 0, only the part under the transition is used
        Undo();
        StartTest("InTransition: With shift: TransitionRightClipBegin: enlarge clip size directly after creating the transition (impossible since right clip cannot be extended further).");
        TimelineShiftTrim(TransitionRightClipBegin(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
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
    TimelineZoomIn(4);
    {
        MakeInOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();
        StartTest("InOutTransition: Without shift: reduce size of clip linked to the out-clip (transition must be removed)");
        TimelineTrim(RightCenter(AudioClip(0,1)),Center(AudioClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the size of the clip linked to the in-clip (transition must be removed)");
        TimelineTrim(LeftCenter(AudioClip(0,2)),Center(AudioClip(0,2)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the size of the clip linked to the out-clip  as much as possible (transition must be removed)");
        TimelineTrim(RightCenter(AudioClip(0,1)),LeftCenter(AudioClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the size of the clip linked to the in-clip as much as possible (transition between videos must be removed)");
        pts lengthOfVideoClipAfterOutClip = VideoClip(0,4)->getLength();
        pts lengthOfAudioClipLinkedToVideoClipAfterOutClip = AudioClip(0,3)->getLength();
        TimelineTrim(LeftCenter(AudioClip(0,2)),RightCenter(AudioClip(0,3)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),lengthOfVideoClipAfterOutClip);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),lengthOfAudioClipLinkedToVideoClipAfterOutClip);
        Undo();
        StartTest("InOutTransition: Without shift: reduce the 'other' side of the clip linked to the in-clip as much as possible (transition is NOT removed)");
        TimelineTrim(LeftCenter(AudioClip(0,1)),RightCenter(AudioClip(0,2)));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(AudioClip )(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_ZERO(VideoClip(0,2)->getLength());
        ASSERT_EQUALS(AudioClip(0,1)->getLength() + AudioClip(0,2)->getLength(), originalLengthOfAudioClip1);
        StartTest("InOutTransition: Without shift: scrub with an 'in' clip which is fully obscured by the transition");
        Scrub(RightPixel(VideoClip(0,1))-5, HCenter(VideoClip(0,3))); // Bug once: The empty video clip that is 'just before' the transition asserted when doing a moveTo(0), since !(0<length) for a clip with length 0.
        StartTest("InOutTransition: Without shift: enlarge 'in' clip which is fully obscured by the transition");
        TimelineTrim(UnderTransitionLeftEdge(VideoClip(0,3)),LeftCenter(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        Undo();
        Undo();
        StartTest("InOutTransition: Without shift: reduce 'other' side of the clip linked to the out-clip as much as possible (transition is NOT removed)");
        TimelineTrim(RightCenter(AudioClip(0,2)),Center(AudioClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_ZERO(VideoClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,2)->getLength() + AudioClip(0,3)->getLength(), originalLengthOfAudioClip2);
        StartTest("InOutTransition: Without shift: scrub an 'out' clip which is fully obscured by the transition");
        Scrub(HCenter(VideoClip(0,2)), LeftPixel(VideoClip(0,4)) + 5); // Bug once: The empty video clip that is 'just before' the transition asserted when doing a moveTo(0), since !(0<length) for a clip with length 0.
        StartTest("InOutTransition: Without shift: enlarge 'out' clip which is fully obscured by the transition");
        TimelineTrim(UnderTransitionRightEdge(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
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
    TimelineZoomIn(4);
    ConfigFixture.SnapToClips(false).SnapToCursor(false);
    {
        MakeInOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();
        StartTest("InOutTransition: Trim left: Reduce size.");
        wxPoint aBitToTheRight = VTopQuarterLeft(VideoClip(0,2)) + wxPoint(20,0);
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),aBitToTheRight);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Trim left: Reduce size and verify upper resize bound.");
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoTransition(0,2)->getLeft());
        ASSERT_ZERO(*(VideoTransition(0,2)->getLeft()));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("InOutTransition: Trim left: Enlarge size and verify lower resize bound (bound imposed by the available extra data in the clip right of the transition).");
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
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
        TimelineTrim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1)) + wxPoint(-10,0));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)            (AudioClip)(AudioClip);
        TimelineTrim(VTopQuarterLeft(VideoClip(0,3)),LeftCenter(VideoClip(0,0)));
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
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),aBitToTheLeft);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InOutTransition: Trim right: Reduce size and verify lower resize bound.");
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoTransition(0,2)->getRight());
        ASSERT_ZERO(*(VideoTransition(0,2)->getRight()));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InOutTransition: Trim right: Enlarge size and verify upper resize bound (bound imposed by the available extra data in the clip left of the transition).");
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
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
        TimelineTrim(RightCenter(VideoClip(0,3)), LeftCenter(VideoClip(0,3)) + wxPoint(10,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(EmptyClip)(AudioClip);
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
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
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),aBitToTheRight);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        StartTest("InTransition: Trim left: Enlarge size (impossible).");
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        StartTest("InTransition: Trim right: Reduce size.");
        wxPoint aBitToTheLeft = VTopQuarterRight(VideoClip(0,2)) - wxPoint(5,0);
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),aBitToTheLeft);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("InTransition: Trim right: Reduce size and verify lower resize bound.");
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),LeftCenter(VideoClip(0,1)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        StartTest("InTransition: Trim right: Enlarge size and verify upper resize bound (bound imposed by the length of the clip right of the transition).");
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
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
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),aBitToTheRight);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), originalLengthOfAudioClip1);
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), originalLengthOfAudioClip2);
        Undo();
        StartTest("OutTransition: Trim left: Reduce size and verify upper resize bound.");
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("OutTransition: Trim left: Enlarge size and verify lower resize bound (bound imposed by the length of the clip left of the transition).");
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_ZERO(VideoClip(0,2)->getMinAdjustBegin());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        StartTest("OutTransition: Trim right: Reduce size (impossible).");
        wxPoint aBitToTheLeft = VTopQuarterRight(VideoClip(0,2)) - wxPoint(20,0);
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),aBitToTheLeft);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)            (AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        StartTest("OutTransition: Trim right: Enlarge size (impossible).");
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),RightCenter(VideoClip(0,3)));
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
        TimelineDeleteClip(VideoClip(0,1));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(Transition)(VideoClip);
        TimelineTrimTransitionRightClipBegin(VideoClip(0,2),20);
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),Center(VideoClip(0,1)));
        Undo(2);
    }
    {
        StartTest("OutTransition: Try enlarging the transition on the right side (known crash in ClipView for finding the correct position)");
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDeleteClip(VideoClip(0,3));
        ASSERT_HISTORY_END(gui::timeline::cmd::DeleteSelectedClips);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip);
        TimelineTrimTransitionLeftClipEnd(VideoClip(0,2),-20);
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),Center(VideoClip(0,4)));
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::DeleteSelectedClips);
        Undo();
    }
    {
        StartTest("InTransition: Try enlarging the transition on the left side (known crash in Transition::getMinAdjustBegin)");
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDeleteClip(VideoClip(0,1));
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(Transition)(VideoClip);
        TimelineTrimTransitionRightClipBegin(VideoClip(0,2),20);
        TimelineTrim(VTopQuarterLeft(VideoClip(0,2)),Center(VideoClip(0,1)));
        Undo(2);
    }
    {
        StartTest("OutTransition: Try enlarging the transition on the right side (known crash in Transition::getMaxAdjustEnd)");
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDeleteClip(VideoClip(0,3));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(EmptyClip);
        TimelineTrimTransitionLeftClipEnd(VideoClip(0,2),-20);
        TimelineTrim(VTopQuarterRight(VideoClip(0,2)),Center(VideoClip(0,4)));
        Undo(2);
    }
}

void TestTransition::testCompletelyTrimmingAwayTransition()
{
    StartTestSuite();
    TimelineZoomIn(4);
    {
        MakeInOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        pts lengthleft = VideoClip(0,1)->getLength();
        pts lengthright = VideoClip(0,3)->getLength();
        wxPoint from = VTopQuarterLeft(VideoClip(0,2));
        TimelineTrim(from,from + wxPoint(100,0));
        from = VTopQuarterRight(VideoClip(0,2));
        TimelineTrim(from,from + wxPoint(-100,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthleft + DefaultTransitionLength() / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthright + DefaultTransitionLength() / 2);
        Undo(2);
    }
    {
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        pts lengthleft = VideoClip(0,1)->getLength();
        pts lengthright = VideoClip(0,3)->getLength();
        wxPoint from = VTopQuarterRight(VideoClip(0,2));
        TimelineTrim(from,from + wxPoint(-100,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthleft);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthright + DefaultTransitionLength() / 2);
        Undo();
    }
    {
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        pts lengthleft = VideoClip(0,1)->getLength();
        pts lengthright = VideoClip(0,3)->getLength();
        wxPoint from = VTopQuarterLeft(VideoClip(0,2));
        TimelineTrim(from,from + wxPoint(100,0));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthleft + DefaultTransitionLength() / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), lengthright);
        Undo();
    }
}

void TestTransition::testSplitNearZeroLengthEdgeOfTransition()
{
    StartTestSuite();
    TimelineZoomIn(5);
    MakeInOutTransitionAfterClip preparation(1);
    {
        StartTest("Left size is 0");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        // Reduce left part of transition to 0
        wxPoint from = VTopQuarterLeft(VideoClip(0,2));
        TimelineTrim(from,from + wxPoint(100,0));
        pts length1 = VideoClip(0,1)->getLength();
        pts length2 = VideoClip(0,2)->getLength();
        pts length3 = VideoClip(0,3)->getLength();
        {
            StartTest("Left size is 0, position the split near, but not ON the cut");
            TimelinePositionCursor(RightPixel(VideoClip(0,1))); // Ensure that the split is done near at the left edge of the transition, but ON the adjacent clip
            TimelineKeyPress('s');
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip); // Transition NOT removed, there is still part of the clip available
            ASSERT_EQUALS(VideoClip(0,1)->getLength() + VideoClip(0,2)->getLength(), length1);
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), length2);
            ASSERT_EQUALS(VideoClip(0,4)->getLength(), length3);
            Undo();
        }
        {
            StartTest("Left size is 0, position the split ON the cut");
            TimelinePositionCursor(LeftPixel(VideoClip(0,2))); // Ensure that the split is done exactly at the left edge of the transition
            TimelineKeyPress('s');
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip); // Transition removed
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), length1);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), length2 + length3); // The clip part under the transition is 'added to the adjacent clip'
            Undo();
        }
        Undo();
    }
    {
        StartTest("Right size is 0");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        // Reduce right part of transition to 0
        wxPoint from = VTopQuarterRight(VideoClip(0,2));
        TimelineTrim(from,from + wxPoint(-100,0));
        pts length1 = VideoClip(0,1)->getLength();
        pts length2 = VideoClip(0,2)->getLength();
        pts length3 = VideoClip(0,3)->getLength();
        {
            StartTest("Right size is 0, position the split near, but not ON the cut");
            TimelinePositionCursor(LeftPixel(VideoClip(0,3))); // Ensure that the split is done near the right edge of the transition, but ON the adjacent clip
            TimelineKeyPress('s');
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip); // Transition NOT removed, there is still part of the clip available
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), length1);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), length2);
            ASSERT_EQUALS(VideoClip(0,3)->getLength() + VideoClip(0,4)->getLength(), length3);
            Undo();
        }
        {
            StartTest("Right size is 0, position the split ON the cut");
            TimelinePositionCursor(TransitionCut(VideoClip(0,2))); // Ensure that the split is done exactly at the right edge of the transition
            TimelineKeyPress('s');
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip); // Transition removed
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), length1 + length2); // The clip part under the transition is 'added to the adjacent clip'
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), length3);
            Undo();
        }
        Undo();
    }
}

void TestTransition::testCreateTransitionAfterLastClip()
{
    StartTestSuite();
    TimelineZoomIn(1); // Zoom in once to avoid clicking in the middle of a clip which is then seen (logically) as clip end due to the zooming
    TimelineTrimRight(VideoClip(0,2), 20);
    {
        StartTest("Create transition after last video clip in track (NOTE: clip is NOT followed by EmptyClip).");
        TimelineDrag(From(Center(VideoClip(0,2))).To(wxPoint(RightPixel(VideoTrack(0)), VCenter(VideoTrack(0)))));
        TimelineMove(wxPoint(RightPixel(VideoTrack(0)), VCenter(VideoTrack(0))));
        TimelineKeyPress('o');
        ASSERT(VideoClip(0,8)->isA<model::Transition>());
        Undo(2);
    }
    {
        StartTest("Create transition after last audio clip in track (NOTE: clip is NOT followed by EmptyClip).");
        TimelineDrag(From(Center(AudioClip(0,2))).To(wxPoint(RightPixel(AudioTrack(0)), VCenter(AudioTrack(0)))));
        TimelineMove(wxPoint(RightPixel(AudioTrack(0)), VCenter(AudioTrack(0))));
        TimelineKeyPress('o');
        ASSERT(AudioClip(0,8)->isA<model::Transition>());
        Undo(2);
    }
}


void TestTransition::testChangeTransitionType()
{
    StartTestSuite();
    TimelineZoomIn(4);
    {
        StartTest("InOutTransitions");
        MakeInOutTransitionAfterClip preparation{ 2 };

        for (model::TransitionPtr transition : model::video::VideoTransitionFactory::get().getAllPossibleTransitionsOfType(model::TransitionTypeFadeOutToNext))
        {
            TimelineSelectClips({ VideoClip(0,3) });
            SetValue(DetailsClipView()->getTransitionTypeSelector(), transition->getDescription(model::TransitionTypeFadeOutToNext));
            ASSERT_EQUALS(VideoClip(0, 3)->getDescription(), transition->getDescription(model::TransitionTypeFadeOutToNext));
            Play(-2 + LeftPixel(VideoClip(0, 3)), 250);
            Undo();
        }
    }
    {
        StartTest("InTransitions");
        MakeInTransitionAfterClip preparation{ 2 };
        for (model::TransitionPtr transition : model::video::VideoTransitionFactory::get().getAllPossibleTransitionsOfType(model::TransitionTypeFadeIn))
        {
            TimelineSelectClips({ VideoClip(0,3) });
            SetValue(DetailsClipView()->getTransitionTypeSelector(), transition->getDescription(model::TransitionTypeFadeIn));
            ASSERT_EQUALS(VideoClip(0, 3)->getDescription(), transition->getDescription(model::TransitionTypeFadeIn));
            Play(-2 + LeftPixel(VideoClip(0, 3)), 250);
            Undo();
        }
    }
    {
        StartTest("OutTransitions");
        MakeOutTransitionAfterClip preparation{ 2 };
        for (model::TransitionPtr transition : model::video::VideoTransitionFactory::get().getAllPossibleTransitionsOfType(model::TransitionTypeFadeOut))
        {
            TimelineSelectClips({ VideoClip(0,3) });
            SetValue(DetailsClipView()->getTransitionTypeSelector(), transition->getDescription(model::TransitionTypeFadeOut));
            ASSERT_EQUALS(VideoClip(0, 3)->getDescription(), transition->getDescription(model::TransitionTypeFadeOut));
            Play(-2 + LeftPixel(VideoClip(0, 3)), 250);
            Undo();
        }
    }
}

void TestTransition::testTransitionPlaybackAndEditTransitionType()
{
    StartTestSuite();
    TimelineZoomIn(4);

    MakeInOutTransitionAfterClip preparation{ 2 };
    TimelineSelectClips({ VideoClip(0,3) });
    StartTest("Preparation");

    WaitForPlaybackStarted started;
    ButtonTriggerPressed(DetailsClipView()->getPlayButton());
    started.wait();

    for (model::TransitionPtr transition : model::video::VideoTransitionFactory::get().getAllPossibleTransitionsOfType(model::TransitionTypeFadeOutToNext))
    {
        wxString description{ transition->getDescription(model::TransitionTypeFadeOutToNext) };
        {
            StartTest("Change to " + description);
            WaitForPlaybackStopped stopped;
            WaitForPlaybackStarted startedagain;
            SetValue(DetailsClipView()->getTransitionTypeSelector(), description);
            stopped.wait();
            startedagain.wait();
            pause(250);
            ASSERT_EQUALS(VideoClip(0, 3)->getDescription(), description);
        }
        {
            StartTest("Undo  " + description);
            WaitForPlaybackStopped stopped;
            WaitForPlaybackStarted startedagain;
            Undo();
            stopped.wait();
            startedagain.wait();
            pause(250);
        }
    }

    // Got crash here when the destructor of  MakeInOutTransitionAfterClip triggered an undo of the transition. 
    // Crash occured in sws_scale (getnextvideo of videofile) in the video buffer thread.
    // Fault was caused by the playback NOT being stopped before undo-ing the transition!
}

void TestTransition::testTransitionPlayback()
{
    StartTestSuite();
    MakeInOutTransitionAfterClip preparation(3);
    TimelineLeftClick(Center(VideoClip(0, 4))); // Open properties
    WaitForPlaybackStarted started;
    ButtonTriggerPressed(DetailsClipView()->getPlayButton(), true);
    started.wait();
    WaitForPlaybackStopped stopped;
    ButtonTriggerPressed(DetailsClipView()->getPlayButton(), false);
    stopped.wait();
}

void TestTransition::testTransitionPlaybackAndEditTransitionParameter()
{
    StartTestSuite();
    TimelineZoomIn(4);
    StartTest("Preparation");

    MakeInOutTransitionAfterClip preparation{ 2 };
    SetValue(DetailsClipView()->getTransitionTypeSelector(), _("Wipe Circle"));
    ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition)(gui::timeline::cmd::EditClipDetails);
    TimelineSelectClips({});
    TimelineSelectClips({ VideoClip(0,3) });

    WaitForPlaybackStarted started;
    ButtonTriggerPressed(DetailsClipView()->getPlayButton());
    started.wait();

    for (int c{ 2 }; c < 30; c += 5)
    {
        {
            StartTest(wxString::Format("Set number of circles to %d", c));
            WaitForPlaybackStopped stopped;
            WaitForPlaybackStarted startedagain;

            setTransitionParameter<model::TransitionParameterInt>(DetailsClipView()->getTransitionClone(), model::TransitionParameterInt::sParameterBandsCount, c);
            stopped.wait();
            startedagain.wait();
            pause(200);
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        }
        {
            StartTest(wxString::Format("Undo number of circles to %d", c));
            WaitForPlaybackStopped stopped;
            WaitForPlaybackStarted startedagain;
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition)(gui::timeline::cmd::EditClipDetails);
            stopped.wait();
            startedagain.wait();
        }
    }

    Undo(); // Set transition type
}

} // namespace
