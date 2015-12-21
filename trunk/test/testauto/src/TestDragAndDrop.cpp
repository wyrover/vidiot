// Copyright 2013-2015 Eric Raijmakers.
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

#include "TestDragAndDrop.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestDragAndDrop::setUp()
{
    mProjectFixture.init();
}

void TestDragAndDrop::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASESMove(
//////////////////////////////////////////////////////////////////////////

void TestDragAndDrop::testStartDrag()
{
    StartTestSuite();
    TimelineLeftClick(Center(VideoClip(0, 1)));
    ASSERT_SELECTION_SIZE(1);
    {
        StartTest("Start drag by CTRL clicking on unselected clip.");
        TimelineDrag(From(Center(VideoClip(0, 2))).HoldCtrlBeforeDragStarts().To(Center(VideoClip(0, 5))).DontReleaseMouse());
        ASSERT_SELECTION_SIZE(2);
        ASSERT(getTimeline().getDrag().isActive());
        TimelineLeftUp(); // End the drag
        ASSERT(!getTimeline().getDrag().isActive());
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 3)); // Test two clips dragged
        Undo();
        ASSERT_SELECTION_SIZE(2);
    }
    {
        StartTest("Start drag by CTRL clicking on already selected clip.");
        TimelineDrag(From(Center(VideoClip(0, 1))).HoldCtrlBeforeDragStarts().To(Center(VideoClip(0, 5))).DontReleaseMouse());
        ASSERT(getTimeline().getDrag().isActive());
        TimelineLeftUp(); // End the drag
        ASSERT(!getTimeline().getDrag().isActive());
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 3)); // Test two clips dragged
        Undo();
        ASSERT_SELECTION_SIZE(2);
    }
    {
        StartTest("Start drag by clicking on already selected clip.");
        TimelineDrag(From(Center(VideoClip(0, 1))).To(Center(VideoClip(0, 5))).DontReleaseMouse());
        ASSERT(getTimeline().getDrag().isActive());
        TimelineLeftUp(); // End the drag
        ASSERT(!getTimeline().getDrag().isActive());
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 3)); // Test two clips dragged
        Undo();
        ASSERT_SELECTION_SIZE(2);
    }
}

void TestDragAndDrop::testDnd()
{
    StartTestSuite();
    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
    TimelineKeyPress('=');  // Zoom in
    {
        StartTest("Dragging: Move one clip around.");
        ConfigFixture.SnapToCursor(true);
        TimelinePositionCursor(0);
        pts length = VideoClip(0,3)->getLength();
        TimelineDrag(From(Center(VideoClip(0,3))).AlignLeft(1)); // Move to a bit after the beginning of timeline, snaps to the cursor
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),length);
        Undo();
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),length );
        ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
        ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
    }
    {
        StartTest("Dragging: Drop a clip with snapping enabled does not affect the clip to the right of the snapping point.");
        ConfigFixture.SnapToClips(true);
        pts lengthOfClipRightOfTheDrop = VideoClip(0,2)->getLength();
        pts lengthOfDroppedClip = VideoClip(0,3)->getLength();
        TimelineDrag(From(Center(VideoClip(0,3))).AlignRight(RightPixel(VideoClip(0,1))));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfClipRightOfTheDrop);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDroppedClip);
        Undo();
        // +wxPoint(): Move the pointer a bit to the right to ensure that the snapping is done one the right edge.
        //             Given the lengths of the VideoClip(0,2) and VideoClip(0,3) - 246 and 250 - the snapping can
        //             easily change from 'the right edge of clip (2)' to 'the left edge of clip (2)', since that's
        //             only a diff of four pts values, which is less than one pixel with the given zoom. When the
        //             mouse pointer is positioned to the right of the center position of the dragged clip, snapping
        //             will first be done on the clip's right edge. The '-2' was added to the target position to
        //             test that the snapping causes an actual extra movement on the drop position.
        TimelineDrag(From(Center(VideoClip(0,3)) + wxPoint(5,0)).AlignRight(LeftPixel(VideoClip(0,2)) - 2));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfClipRightOfTheDrop);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDroppedClip);
        Undo();
    }
    {
        StartTest("Dragging: Move one clip partially on top of its original location (caused a recursion error in AClipEdit, for expanding the replacement map).");
        pts length = VideoClip(0,3)->getLength();
        TimelineDrag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,3)) + wxPoint(20,0))); // Move the clip only a bit to the right
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),length );
        Undo();
    }
    {
        StartTest("Dragging: Move a clip beyond the track length.");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        TimelineDrag(From(Center(VideoClip(0,1))).To(wxPoint(RightPixel(VideoClip(0,6)) + VideoClip(0,1)->getLength(), VCenter(VideoClip(0,1))))); // + + VideoClip(0,1)->getLength(): Ensure that it's dropped after a bit of empty space
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,8)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,8)->getLink(),AudioClip(0,8));
        ASSERT_EQUALS(AudioClip(0,8)->getLink(),VideoClip(0,8));
        Undo();
    }
    {
        StartTest("Dragging: With Scrolling Offset: If dragged not close enough to a clip boundary, no snapping is applied.");
        getTimeline().Scroll(80,0);
        ConfigFixture.SnapToClips(true).SnapToCursor(false);
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,4)) + 25));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        Undo();
    }
    {
        StartTest("Dragging: With Scrolling Offset: If dragged close enough to a clip boundary, snapping is applied.");
        getTimeline().Scroll(80,0);
        ConfigFixture.SnapToClips(true).SnapToCursor(false);
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,4)) + 5));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        Undo();
    }
    {
        StartTest("Dragging: With Scrolling Offset: If dragged close enough to a clip boundary, but snapping is disabled, no snapping is applied.");
        getTimeline().Scroll(80,0);
        ConfigFixture.SnapToClips(false).SnapToCursor(false);
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,4)) + 5));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        Undo();
    }
}

void TestDragAndDrop::testDndMultipleTracks()
{
    StartTestSuite();
    TimelineZoomIn(2);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    {
        StartTest("Known bug at one point: Clip removed when using CTRL to change drag point.");
        TimelineDragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        wxPoint inbetween(HCenter(VideoClip(0,4)),VCenter(VideoTrack(1)));
        TimelineDrag(From(Center(VideoClip(1,1))).To(inbetween).DontReleaseMouse());
        TimelineKeyDown(WXK_CONTROL);
        TimelineDrag(From(Center(VideoClip(0,4))).To(Center(VideoClip(0,5))).DontReleaseMouse());
        ASSERT(getTimeline().getDrag().isActive());
        TimelineKeyUp(WXK_CONTROL);
        TimelineDrag(From(Center(VideoClip(0,5))).To(inbetween));
        ASSERT_VIDEOTRACK2(EmptyClip)                      (VideoClip); // Bug occurred here: VideoClip was moved to track2, which did not exist
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip); // then, because the 'add track during drag' was not implemented then yet.
        ASSERT_VIDEOTRACKS(3);
        ASSERT_AUDIOTRACKS(2);
        ASSERT_VIDEOTRACK2SIZE(2);
        ASSERT_VIDEOTRACK1SIZE(0);
        ASSERT_AUDIOTRACK1SIZE(2);
        Undo(2);
    }
    {
        StartTest("Known bug at one point: Clip removed when using CTRL to change drag point.");
        TimelineDragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        wxPoint inbetween(HCenter(AudioClip(0,4)),VCenter(AudioTrack(1)));
        TimelineDrag(From(Center(AudioClip(1,1))).To(inbetween).DontReleaseMouse());
        TimelineKeyDown(WXK_CONTROL);
        TimelineDrag(From(Center(AudioClip(0,4))).To(Center(AudioClip(0,5))).DontReleaseMouse());
        TimelineKeyUp(WXK_CONTROL);
        TimelineDrag(From(Center(AudioClip(0,5))).To(inbetween));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip); // Bug occurred here: AudioClip was moved to track2, which did not exist
        ASSERT_AUDIOTRACK2(EmptyClip)                      (AudioClip); // then, because the 'add track during drag' was not implemented then yet.
        ASSERT_VIDEOTRACKS(2);
        ASSERT_AUDIOTRACKS(3);
        ASSERT_VIDEOTRACK1SIZE(2);
        ASSERT_AUDIOTRACK1SIZE(0);
        ASSERT_AUDIOTRACK2SIZE(2);
        Undo(2);
    }
}

void TestDragAndDrop::testDndRightMouseScrolling()
{
    StartTestSuite();
    TimelineZoomIn(6);
    {
        StartTest("Left to right");
        pts length{ VideoClip(0, 0)->getLength() };
        wxPoint inbetween(Center(VideoClip(0,1)));
        TimelineDrag(From(Center(VideoClip(0,0))).To(inbetween).DontReleaseMouse());
        for (int i = 0; i < 3; ++i) // Right mouse scroll such that the original clip's position is no longer visible.
        {
            TimelineRightMouseScroll(-300);
        }
        TimelineMove(wxPoint(getTimeline().getScrolling().getOffset().x + getTimeline().GetSize().GetWidth() - 100, inbetween.y));
        TimelineLeftUp(); // End drag
        ASSERT_VIDEOTRACK0(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0, 8)->getLength(), length);
        Undo(1);
    }
    {
        StartTest("Right to left");
        for (int i = 0; i < 3; ++i) // Move to the right
        {
            TimelineRightMouseScroll(-300);
        }
        pts length{ VideoClip(0, 6)->getLength() };
        wxPoint inbetween(Center(VideoClip(0,5)));
        TimelineDrag(From(Center(VideoClip(0,6))).To(inbetween).DontReleaseMouse());
        for (int i = 0; i < 3; ++i) // Left mouse scroll such that the original clip's position is no longer visible.
        {
            TimelineRightMouseScroll(300);
        }
        TimelineMove(wxPoint(2, inbetween.y));
        TimelineLeftUp(); // End drag
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_VIDEOTRACK0SIZE(5);
        ASSERT_EQUALS(VideoClip(0, 0)->getLength(), length);
        Undo(1);
    }
}

void TestDragAndDrop::testSnapping()
{
    StartTestSuite();
    TimelineZoomIn(4);
    TimelinePositionCursor(LeftPixel(VideoClip(0,2)) + 10); // Just a bit to the right of the leftmost point of this clip
    ConfigFixture.SnapToClips(false).SnapToCursor(true);
    {
        StartTest("No snapping when dragged beyond snap distance");
        TimelineDrag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::timeline::Timeline::SnapDistance + 1).DontReleaseMouse());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        TimelineLeftUp();
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Undo();
    }
    {
        StartTest("Snap to cursor when inside snap distance");
        TimelineDrag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::timeline::Timeline::SnapDistance - 1).DontReleaseMouse());
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        TimelineLeftUp();
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset()); // reset
        Undo();
    }
    {
        StartTest("Temporarily disable snapping");
        TimelineDrag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::timeline::Timeline::SnapDistance - 1).DontReleaseMouse());
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        TimelineKeyPress('d'); // disable snapping
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        TimelineKeyPress('d'); // enable snapping
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        TimelineKeyPress('d'); // disable snapping
        TimelineLeftUp();
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Undo();
    }

}

void TestDragAndDrop::testDropAdjacentToTransition()
{
    StartTestSuite();
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,true);

    TimelineZoomIn(4);
    {
        StartTest("InOutTransition: Drop adjacent to left edge");
        MakeInOutTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelinePositionCursor(10); 
        TimelineDrag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        TimelinePositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("InOutTransition: Drop adjacent to right edge");
        MakeInOutTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelinePositionCursor(10); 
        TimelineDrag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        TimelinePositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
        TimelineSelectClips({});
    }
    {
        StartTest("InTransition: Drop adjacent to right edge");
        MakeInTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelinePositionCursor(10); 
        TimelineDrag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        TimelinePositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("OutTransition: Drop adjacent to left edge");
        MakeOutTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelinePositionCursor(10); 
        TimelineDrag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        TimelinePositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("InOutTransition with 0-length clips: Drop adjacent to left edge");
        MakeInOutTransitionAfterClip prepare(2);
        TimelineTrimLeft(VideoClip(0,2), 300);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelinePositionCursor(10); 
        TimelineDrag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        TimelinePositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo(2);
    }
    {
        StartTest("InOutTransition with 0-length clips: Drop adjacent to right edge");
        MakeInOutTransitionAfterClip prepare(2);
        TimelineTrimRight(VideoClip(0,4), -200); 
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelinePositionCursor(10); 
        TimelineDrag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        TimelinePositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo(2);
    }
}

void TestDragAndDrop::testDragZeroLengthSideOfTransition()
{
    StartTestSuite();
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);
    TimelineZoomIn(4);
    TimelinePositionCursor(10);
    MakeInOutTransitionAfterClip preparation(2);
    {
        StartTest("Left size is 0, drag left clip");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition);
        MakeVideoTransitionLeftPart0(0,3);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDrag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,5))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Right size is 0, drag right clip");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition);
        MakeVideoTransitionRightPart0(0,3);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDrag(From(Center(VideoClip(0,4))).AlignRight(RightPixel(VideoClip(0,5))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip);
        Undo(2);
    }
}

void TestDragAndDrop::testDropZeroLengthSideOfTransition()
{
    StartTestSuite();
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);
    TimelineZoomIn(4);
    TimelinePositionCursor(10);
    MakeInOutTransitionAfterClip preparation(2);
    {
        StartTest("Left size is 0, drop on cut");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition);
        MakeVideoTransitionLeftPart0(0,3);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDrag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Left size is 0, drop besides cut (a bit to the left)");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition);
        MakeVideoTransitionLeftPart0(0,3);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDrag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3)) - 4));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Right size is 0, drop on cut");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition);
        MakeVideoTransitionRightPart0(0,3);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDrag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Right size is 0, drop besides cut (a bit to the right)");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition);
        MakeVideoTransitionRightPart0(0,3);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineDrag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3)) + 4));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
}

void TestDragAndDrop::testStartDragNearDividerEdge()
{
    StartTestSuite();
    TimelineZoomIn(4);
    {
        StartTest("Video top to video track divider");
        wxPoint startingPoint{ HCenter(VideoClip(0, 2)), TopPixel(VideoClip(0, 2)) + 1 }; // +1: Otherwise, the begin position is on the bottom of the divider, not on the top of the clip.
        wxPoint viaPoint{ startingPoint };
        viaPoint.x += gui::timeline::Drag::Threshold + 10; // Ensure that drag is started by moving enough in the horizontal direction.
        viaPoint.y -= 2; // Ensure that the mouse pointer 'ends up' inside the top track divider
        ASSERT(getTimeline().getMouse().getInfo(viaPoint).onTrackDivider);
        TimelineDrag(From(startingPoint).Via(viaPoint).To(Center(VideoClip(0, 4))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        Undo();
    }
    {
        StartTest("Video bottom to audio video divider");
        wxPoint startingPoint{ HCenter(VideoClip(0, 2)), BottomPixel(VideoClip(0, 2)) };
        wxPoint viaPoint{ startingPoint };
        viaPoint.x += gui::timeline::Drag::Threshold + 10; // Ensure that drag is started by moving enough in the horizontal direction.
        viaPoint.y++; // Ensure that the mouse pointer 'ends up' inside the audio-video divider
        ASSERT(getTimeline().getMouse().getInfo(viaPoint).onAudioVideoDivider);
        TimelineDrag(From(startingPoint).Via(viaPoint).To(Center(VideoClip(0, 4))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        Undo();
    }
    {
        StartTest("Audio top to audio video divider");
        wxPoint startingPoint{ HCenter(AudioClip(0, 2)), TopPixel(AudioClip(0, 2)) + 1 }; // +1: Otherwise, the begin position is on the bottom of the divider, not on the top of the clip.
        wxPoint viaPoint{ startingPoint };
        viaPoint.x += gui::timeline::Drag::Threshold + 10; // Ensure that drag is started by moving enough in the horizontal direction.
        viaPoint.y -= 2; // Ensure that the mouse pointer 'ends up' inside the audio-video divider
        ASSERT(getTimeline().getMouse().getInfo(viaPoint).onAudioVideoDivider);
        TimelineDrag(From(startingPoint).Via(viaPoint).To(Center(AudioClip(0, 4))));
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        Undo();
    }
    {
        StartTest("Audio bottom to audio track divider");
        wxPoint startingPoint{ HCenter(AudioClip(0, 2)), BottomPixel(AudioClip(0, 2)) };
        wxPoint viaPoint{ startingPoint };
        viaPoint.x += gui::timeline::Drag::Threshold + 10; // Ensure that drag is started by moving enough in the horizontal direction.
        viaPoint.y++; // Ensure that the mouse pointer 'ends up' inside the bottom track divider
        ASSERT(getTimeline().getMouse().getInfo(viaPoint).onTrackDivider);
        TimelineDrag(From(startingPoint).Via(viaPoint).To(Center(VideoClip(0, 4))));
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        Undo();
    }
}

void TestDragAndDrop::testCreateTrackDuringDraggingByMovingMouseOutsideListOfTracks()
{
    StartTestSuite();
    ASSERT_VIDEOTRACKS(1);
    ASSERT_AUDIOTRACKS(1);
    {
        StartTest("Add video track");
        wxPoint start{ Center(VideoClip(0, 2)) };
        wxPoint to{ start.x, getTimeline().getSequenceView().getVideo().getRect().GetTop() };
        TimelineDrag(From(start).To(to).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(1);
        to.y--;
        TimelineDrag(To(to).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(2);
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
        ASSERT_ZERO(NumberOfVideoClipsInTrack(1)); // They're part of the drag, not of the sequence yet.
        StartTest("Only one track is added");
        TimelineDrag(To(to).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(2);
        StartTest("Execute the drop");
        TimelineDrag(To(to));
        ASSERT_VIDEOTRACKS(2);
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
        ASSERT_VIDEOTRACK1(     EmptyClip      )(VideoClip);
        StartTest("Undo add video track");
        Undo();
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
        ASSERT_VIDEOTRACKS(1);
        StartTest("Redo add video track");
        Redo();
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
        ASSERT_VIDEOTRACKS(2);
        ASSERT_VIDEOTRACK1(     EmptyClip      )(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(1, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(1, 1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_AUDIOTRACKS(1);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Undo();
    }
    {
        StartTest("Add audio track");
        wxPoint start{ Center(AudioClip(0, 2)) };
        wxPoint to{ start.x, getTimeline().getSequenceView().getAudio().getRect().GetBottom() };
        TimelineDrag(From(start).To(to).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(1);
        to.y++;
        TimelineDrag(To(to).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(2);
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
        ASSERT_ZERO(NumberOfAudioClipsInTrack(1)); // They're part of the drag, not of the sequence yet.
        StartTest("Only one track is added");
        TimelineDrag(To(to).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(2);
        StartTest("Execute the drop");
        TimelineDrag(To(to));
        ASSERT_AUDIOTRACKS(2);
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
        ASSERT_AUDIOTRACK1(     EmptyClip      )(AudioClip);
        StartTest("Undo add audio track");
        Undo();
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
        ASSERT_AUDIOTRACKS(1);
        StartTest("Redo add audio track");
        Redo();
        ASSERT_HISTORY_END(gui::timeline::cmd::ExecuteDrop);
        ASSERT_AUDIOTRACKS(2);
        ASSERT_AUDIOTRACK1(     EmptyClip      )(AudioClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(AudioClip(1, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(1, 1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_VIDEOTRACKS(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        Undo();
    }
    ASSERT_VIDEOTRACKS(1);
    ASSERT_AUDIOTRACKS(1);
}

void TestDragAndDrop::testCreateTrackDuringDraggingByMovingOutwardMaximally()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    wxPoint p{ Center(VideoClip(0,1)) };
    TimelineDrag(From(Center(VideoClip(0,1))).To(wxPoint(p.x, VCenter(VideoTrack(1)))));
    TimelineDrag(From(Center(AudioClip(0,1))).To(wxPoint(p.x, VCenter(AudioTrack(1)))));
    ASSERT_VIDEOTRACKS(2);
    ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip);
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_AUDIOTRACKS(2);
    ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip);
    ASSERT_AUDIOTRACK1(EmptyClip)(AudioClip);

    {
        StartTest("Video: Drag first clip. First clip in lower track. Check upper bound and add track.");
        TimelineSelectClips({ VideoClip(0, 0), VideoClip(1, 1) });
        wxPoint p{ Center(VideoClip(0,0)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(VideoTrack(1)))).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(3);
        TimelineDrag(To(wxPoint(p.x, getTimeline().getSequenceView().getVideo().getRect().GetTop() - 2)).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(3); // Verify only one track is added
        TimelineDrag(To(wxPoint(p.x, VCenter(VideoTrack(0)))).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(2); // 'Added track' removed when no longer needed
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(VideoTrack(1)))));
        ASSERT_VIDEOTRACKS(3);
        Undo();
        ASSERT_VIDEOTRACKS(2);
        Redo();
        ASSERT_VIDEOTRACKS(3);
        ASSERT_VIDEOTRACK2(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK1(VideoClip);
        ASSERT_VIDEOTRACK0(      EmptyClip      )(VideoClip);
        ASSERT_VIDEOTRACK2SIZE(2);
        ASSERT_VIDEOTRACK1SIZE(1);
        ASSERT_EQUALS(VideoClip(2, 0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(2, 1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(1, 0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0,2));
        Undo();
    }
    {
        StartTest("Video: Drag second clip. First clip in lower track. Check lower bound.");
        TimelineSelectClips({ VideoClip(0, 0), VideoClip(1, 1) });
        wxPoint p{ Center(VideoClip(1,1)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(VideoTrack(0)))));
        ASSERT_VIDEOTRACKS(2);
        ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
    }
    {
        StartTest("Video: Drag first clip. Second clip in lower track. Check lower bound.");
        TimelineSelectClips({ VideoClip(1, 1), VideoClip(0, 2) });
        wxPoint p{ Center(VideoClip(1,1)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(VideoTrack(0)))));
        ASSERT_VIDEOTRACKS(2);
        ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
    }
    {
        StartTest("Video: Drag second clip. Second clip in lower track.");
        TimelineSelectClips({ VideoClip(1, 1), VideoClip(0, 2) });
        wxPoint p{ Center(VideoClip(0,2)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(VideoTrack(1)))).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(3);
        TimelineDrag(To(wxPoint(p.x, getTimeline().getSequenceView().getVideo().getRect().GetTop() - 2)).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(3); // Verify only one track is added
        TimelineDrag(To(wxPoint(p.x, VCenter(VideoTrack(0)))).DontReleaseMouse());
        ASSERT_VIDEOTRACKS(2); // 'Added track' removed when no longer needed
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(VideoTrack(1)))));
        ASSERT_VIDEOTRACKS(3);
        Undo();
        ASSERT_VIDEOTRACKS(2);
        Redo();
        ASSERT_VIDEOTRACKS(3);
        ASSERT_VIDEOTRACK2(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK1(      EmptyClip     )(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(       EmptyClip     )(VideoClip);
        ASSERT_VIDEOTRACK2SIZE(2);
        ASSERT_VIDEOTRACK1SIZE(2);
        ASSERT_EQUALS(VideoClip(2, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(2, 1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(1, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(1, 1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0, 2)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0,3));
        Undo();
    }
    {
        StartTest("Audio: Drag first clip. First clip in lower track. Check upper bound and add track.");
        TimelineSelectClips({ AudioClip(0, 0), AudioClip(1, 1) });
        wxPoint p{ Center(AudioClip(0,0)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(AudioTrack(1)))).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(3);
        TimelineDrag(To(wxPoint(p.x, getTimeline().getSequenceView().getAudio().getRect().GetBottom() + 2)).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(3); // Verify only one track is added
        TimelineDrag(To(wxPoint(p.x, VCenter(AudioTrack(0)))).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(2); // 'Added track' removed when no longer needed
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(AudioTrack(1)))));
        ASSERT_AUDIOTRACKS(3);
        Undo();
        ASSERT_AUDIOTRACKS(2);
        Redo();
        ASSERT_AUDIOTRACKS(3);
        ASSERT_AUDIOTRACK2(EmptyClip)(AudioClip);
        ASSERT_AUDIOTRACK1(AudioClip);
        ASSERT_AUDIOTRACK0(      EmptyClip      )(AudioClip);
        ASSERT_AUDIOTRACK2SIZE(2);
        ASSERT_AUDIOTRACK1SIZE(1);
        ASSERT_EQUALS(AudioClip(2, 0)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,0));
        ASSERT_EQUALS(AudioClip(2, 1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(1, 0)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,0));
        ASSERT_EQUALS(AudioClip(0, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0,2));
        Undo();
    }
    {
        StartTest("Video: Drag second clip. First clip in lower track. Check lower bound.");
        TimelineSelectClips({ AudioClip(0, 0), AudioClip(1, 1) });
        wxPoint p{ Center(AudioClip(1,1)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(AudioTrack(0)))));
        ASSERT_AUDIOTRACKS(2);
        ASSERT_AUDIOTRACK1(EmptyClip)(AudioClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip);
    }
    {
        StartTest("Video: Drag first clip. Second clip in lower track. Check lower bound.");
        TimelineSelectClips({ AudioClip(1, 1), AudioClip(0, 2) });
        wxPoint p{ Center(AudioClip(1,1)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(AudioTrack(0)))));
        ASSERT_AUDIOTRACKS(2);
        ASSERT_AUDIOTRACK1(EmptyClip)(AudioClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip);
    }
    {
        StartTest("Video: Drag second clip. Second clip in lower track.");
        TimelineSelectClips({ AudioClip(1, 1), AudioClip(0, 2) });
        wxPoint p{ Center(AudioClip(0,2)) };
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(AudioTrack(1)))).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(3);
        TimelineDrag(To(wxPoint(p.x, getTimeline().getSequenceView().getAudio().getRect().GetBottom() + 2)).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(3); // Verify only one track is added
        TimelineDrag(To(wxPoint(p.x, VCenter(AudioTrack(0)))).DontReleaseMouse());
        ASSERT_AUDIOTRACKS(2); // 'Added track' removed when no longer needed
        TimelineDrag(From(p).To(wxPoint(p.x, VCenter(AudioTrack(1)))));
        ASSERT_AUDIOTRACKS(3);
        Undo();
        ASSERT_AUDIOTRACKS(2);
        Redo();
        ASSERT_AUDIOTRACKS(3);
        ASSERT_AUDIOTRACK2(EmptyClip)(AudioClip);
        ASSERT_AUDIOTRACK1(      EmptyClip     )(AudioClip);
        ASSERT_AUDIOTRACK0(AudioClip)(       EmptyClip     )(AudioClip);
        ASSERT_AUDIOTRACK2SIZE(2);
        ASSERT_AUDIOTRACK1SIZE(2);
        ASSERT_EQUALS(AudioClip(2, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(2, 1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(1, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(1, 1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0, 1)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0, 2)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0,3));
        Undo();
    }
}

} // namespace