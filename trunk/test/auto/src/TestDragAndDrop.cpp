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

#include "TestDragAndDrop.h"

#include "CreateTransition.h"
#include "Cursor.h"
#include "Drag.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "Timeline.h"

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

void TestDragAndDrop::testDnd()
{
    StartTestSuite();
    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
    Type('=');  // Zoom in
    {
        StartTest("Dragging: Move one clip around.");
        ConfigFixture.SnapToCursor(true);
        pts length = VideoClip(0,3)->getLength();
        Drag(From(Center(VideoClip(0,3))).AlignLeft(1)); // Move to a bit after the beginning of timeline, snaps to the cursor
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),length);
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
        Drag(From(Center(VideoClip(0,3))).AlignRight(RightPixel(VideoClip(0,1))));
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
        Drag(From(Center(VideoClip(0,3)) + wxPoint(5,0)).AlignRight(LeftPixel(VideoClip(0,2)) - 2));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfClipRightOfTheDrop);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDroppedClip);
        Undo();
    }
    {
        StartTest("Dragging: Move one clip partially on top of its original location (caused a recursion error in AClipEdit, for expanding the replacement map).");
        pts length = VideoClip(0,3)->getLength();
        Drag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,3)) + wxPoint(20,0))); // Move the clip only a bit to the right
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),length );
        Undo();
    }
    {
        StartTest("Dragging: Move a clip beyond the track length.");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Drag(From(Center(VideoClip(0,1))).To(wxPoint(RightPixel(VideoClip(0,6)) + VideoClip(0,1)->getLength(), VCenter(VideoClip(0,1))))); // + + VideoClip(0,1)->getLength(): Ensure that it's dropped after a bit of empty space
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
        DeselectAllClips();
        Drag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,4)) + 25));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        Undo();
    }
    {
        StartTest("Dragging: With Scrolling Offset: If dragged close enough to a clip boundary, snapping is applied.");
        getTimeline().Scroll(80,0);
        ConfigFixture.SnapToClips(true).SnapToCursor(false);
        DeselectAllClips();
        Drag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,4)) + 5));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        Undo();
    }
    {
        StartTest("Dragging: With Scrolling Offset: If dragged close enough to a clip boundary, but snapping is disabled, no snapping is applied.");
        getTimeline().Scroll(80,0);
        ConfigFixture.SnapToClips(false).SnapToCursor(false);
        DeselectAllClips();
        Drag(From(Center(VideoClip(0,2))).AlignLeft(LeftPixel(VideoClip(0,4)) + 5));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
        Undo();
    }
}

void TestDragAndDrop::testDndMultipleTracks()
{
    StartTestSuite();
    Zoom level(2);
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);
    {
        StartTest("Known bug at one point: Clip removed when using CTRL to change drag point.");
        DragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        wxPoint inbetween(HCenter(VideoClip(0,4)),VCenter(VideoTrack(1)));
        Drag(From(Center(VideoClip(1,1))).To(inbetween).DontReleaseMouse());
        ControlDown();
        Drag(From(Center(VideoClip(0,4))).To(Center(VideoClip(0,5))).DontReleaseMouse());
        ASSERT(getTimeline().getDrag().isActive());
        ControlUp();
        Drag(From(Center(VideoClip(0,5))).To(inbetween));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip); // Bug occurred here: VideoClip was moved to track2 (which did not exist)
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        Undo(2);
    }
    {
        StartTest("Known bug at one point: Clip removed when using CTRL to change drag point.");
        DragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        wxPoint inbetween(HCenter(AudioClip(0,4)),VCenter(AudioTrack(1)));
        Drag(From(Center(AudioClip(1,1))).To(inbetween).DontReleaseMouse());
        ControlDown();
        Drag(From(Center(AudioClip(0,4))).To(Center(AudioClip(0,5))).DontReleaseMouse());
        ControlUp();
        Drag(From(Center(AudioClip(0,5))).To(inbetween));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip); // Bug occurred here: AudioClip was moved to track2 (which did not exist)
        Undo(2);
    }
}

void TestDragAndDrop::testSnapping()
{
    StartTestSuite();
    Zoom level(6);
    PositionCursor(LeftPixel(VideoClip(0,2)) + 10); // Just a bit to the right of the leftmost point of this clip
    ConfigFixture.SnapToClips(false).SnapToCursor(true);
    {
        StartTest("No snapping when dragged beyond snap distance");
        Drag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::Layout::SnapDistance + 1).DontReleaseMouse());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        LeftUp();
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Undo();
    }
    {
        StartTest("Snap to cursor when inside snap distance");
        Drag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::Layout::SnapDistance - 1).DontReleaseMouse());
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        LeftUp();
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset()); // reset
        Undo();
    }
    {
        StartTest("Temporarily disable snapping");
        Drag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::Layout::SnapDistance - 1).DontReleaseMouse());
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        Type('d'); // disable snapping
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Type('d'); // enable snapping
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        Type('d'); // disable snapping
        LeftUp();
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Undo();
    }

}

void TestDragAndDrop::testDropAdjacentToTransition()
{
    StartTestSuite();
    ConfigOverruleBool overruleSnapToCursor(Config::sPathSnapClips,false);
    ConfigOverruleBool overruleSnapToClips(Config::sPathSnapCursor,true);

    Zoom level(2);
    {
        StartTest("InOutTransition: Drop adjacent to left edge");
        MakeInOutTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        PositionCursor(10); 
        Drag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        PositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("InOutTransition: Drop adjacent to right edge");
        MakeInOutTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        PositionCursor(10); 
        Drag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        PositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("InTransition: Drop adjacent to right edge");
        MakeInTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        PositionCursor(10); 
        Drag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        PositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("OutTransition: Drop adjacent to left edge");
        MakeOutTransitionAfterClip prepare(2);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        PositionCursor(10); 
        Drag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        PositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo();
    }
    {
        StartTest("InOutTransition with 0-length clips: Drop adjacent to left edge");
        MakeInOutTransitionAfterClip prepare(2);
        TrimLeft(VideoClip(0,2), 300);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        PositionCursor(10); 
        Drag(From(Center(VideoClip(0,7))).AlignRight(LeftPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        PositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo(2);
    }
    {
        StartTest("InOutTransition with 0-length clips: Drop adjacent to right edge");
        MakeInOutTransitionAfterClip prepare(2);
        TrimRight(VideoClip(0,4), -200); 
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        PositionCursor(10); 
        Drag(From(Center(VideoClip(0,7))).AlignLeft(RightPixel(VideoClip(0,3))));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        PositionCursor(HCenter(VideoClip(0,2))); // try to get one frame from the transition (was not removed when the error occurred)
        Undo(2);
    }
}

} // namespace