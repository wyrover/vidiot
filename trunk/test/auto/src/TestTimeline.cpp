#include "TestTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "Zoom.h"

namespace test {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

auto PrepareSnapping = [](bool enableSnapping)
{
    checkMenu(ID_SNAP_CLIPS, enableSnapping);
    checkMenu(ID_SNAP_CURSOR, enableSnapping);
    DeselectAllClips();
};

void TestTimeline::setUp()
{
    mProjectFixture.init();
    PrepareSnapping(true);
}

void TestTimeline::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTimeline::testSelection()
{
    const model::IClips& clips = getSequence()->getVideoTrack(0)->getClips();
    {
        // Test CTRL clicking all clips one by one
        ControlDown();
        BOOST_FOREACH(model::IClipPtr clip, clips)
        {
            Click(Center(clip));
        }
        ControlUp();
        ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    }
    {
        // Test SHIFT clicking the entire list
        DeselectAllClips();
        ASSERT_SELECTION_SIZE(0);
        ShiftDown();
        Click(Center(clips.front()));
        Click(Center(clips.back()));
        ShiftUp();
        ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    }
    {
        // Test SHIFT clicking only the partial list
        DeselectAllClips();
        ASSERT_SELECTION_SIZE(0);
        Click(Center(VideoClip(0,2)));
        ShiftDown();
        Click(Center(VideoClip(0,4)));
        ShiftUp();
        ASSERT_SELECTION_SIZE(3);
    }
    {
        // Test (de)selecting one clip with CTRL click
        ControlDown();
        Click(Center(VideoClip(0,3)));
        ControlUp();
        ASSERT_SELECTION_SIZE(2);
        ControlDown();
        Click(Center(VideoClip(0,3)));
        ControlUp();
        ASSERT_SELECTION_SIZE(3);
    }
    {
        // Test selection the transition between two clips when shift selecting
        DeselectAllClips();
        MakeInOutTransitionAfterClip preparation(1);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        Click(Center(VideoClip(0,1)));
        ShiftDown();
        Click(Center(VideoClip(0,3)));
        ShiftUp();
        ASSERT(VideoClip(0,2)->isA<model::Transition>() && VideoClip(0,2)->getSelected());
    }
    // Zoom in (required for correct positioning)
    Type('=');
    Type('=');
    Type('=');
    Type('=');
    {
        // Selection tests for an in-out-transition
        MakeInOutTransitionAfterClip preparation(1);
        // Clicking on TransitionLeftClipInterior selects the clip left of the transition.
        DeselectAllClips();
        Click(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        // Clicking on TransitionLeftClipEnd selects the clip left of the transition.
        DeselectAllClips();
        Click(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        // Clicking on TransitionRightClipInterior selects the clip right of the transition.
        DeselectAllClips();
        Click(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,3)->getSelected());
        // Clicking on TransitionRightClipBegin selects the clip right of the transition.
        DeselectAllClips();
        Click(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(VideoClip(0,3)->getSelected());
    }
    {
        // Selection tests for an out-only-transition
        MakeOutTransitionAfterClip preparation(1);
        // Clicking on TransitionLeftClipInterior selects the clip left of the transition.
        DeselectAllClips();
        Click(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        // Clicking on TransitionLeftClipEnd selects the clip left of the transition.
        DeselectAllClips();
        Click(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
    }
    {
        // Selection tests for an in-only-transition
        MakeInTransitionAfterClip preparation(1);
        // Clicking on TransitionRightClipInterior selects the clip right of the transition.
        DeselectAllClips();
        Click(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,3)->getSelected());
        // Clicking on TransitionRightClipBegin selects the clip right of the transition.
        DeselectAllClips();
        Click(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(VideoClip(0,3)->getSelected());
    }
}

void TestTimeline::testDnd()
{
    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
    Type('=');  // Zoom in
    {
        // Test moving one clip around
        PrepareSnapping(true);
        pts length = VideoClip(0,3)->getLength();
        DragAlignLeft(Center(VideoClip(0,3)),1); // Move to a bit after the beginning of timeline
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
        // Test that dropping a clip with snapping enabled does not affect the clip to the right of the snapping point.
        PrepareSnapping(true);
        pts lengthOfClipRightOfTheDrop = VideoClip(0,2)->getLength();
        pts lengthOfDroppedClip = VideoClip(0,3)->getLength();
        DragAlignRight(Center(VideoClip(0,3)),RightPixel(VideoClip(0,1)));
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
        DragAlignRight(Center(VideoClip(0,3)) + wxPoint(5,0),LeftPixel(VideoClip(0,2)) - 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfClipRightOfTheDrop);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDroppedClip);
        Undo();
    }
}

void TestTimeline::testDnDTransition()
{
    // Zoom in
    Type('=');
    Type('=');
    Type('=');
    MakeInOutTransitionAfterClip preparation(1);

    {
        // Shift drag without snapping enabled,
        // transition and its adjacent clips are shifted backwards
        PrepareSnapping(false);
        ShiftDrag(Center(VideoClip(0,6)),Center(VideoClip(0,3)));
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,4)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        // Shift drag without snapping enabled (drop over clip AFTER
        // transition) clip after transition is shifted backwards ->
        // transition is removed because the two 'transitioned clips
        // are separated'. (clip in front of transition remains intact)
        PrepareSnapping(false);
		ShiftDrag(Center(VideoClip(0,6)),Center(VideoClip(0,4)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that
        // the left position of the drop is aligned with the left
        // position of the clip left of the transitions
        PrepareSnapping(true);
        pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
        ShiftDragAlignLeft(Center(VideoClip(0,6)),preparation.leftPositionOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that the left
        // position of the drop is aligned with the left position of
        // the transition. This causes the clip left of the transition
        // to be shifted back.
        PrepareSnapping(true);
        pts lengthOfDraggedClip = VideoClip(0,5)->getLength();
        ShiftDragAlignLeft(Center(VideoClip(0,5)),preparation.leftPositionOfTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,4)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,5)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that the left position@
        // of the drop is aligned with the center position of the transition.
        // This causes the clip right of the transition to be shifted back, and the transition
        // to be removed.
        PrepareSnapping(true);
        pts lengthOfDraggedClip = VideoClip(0,5)->getLength();
        ShiftDragAlignLeft(Center(VideoClip(0,5)),preparation.touchPositionOfTransition);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
    }
    {
        // Shift drag with snapping enabled. The drop is done such that the left position of the drop is aligned
        // with the left position of the clip after the transition.
        // This causes the clip right of the transition to be shifted back, and the transition
        // to be removed.
        PrepareSnapping(true);
        pts lengthOfDraggedClip = VideoClip(0,5)->getLength();
        ShiftDragAlignLeft(Center(VideoClip(0,5)),LeftPixel(VideoClip(0,3)));
        ASSERT_EQUALS(VideoClip(0,0)->getLength(),preparation.lengthOfFirstClip);
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), lengthOfDraggedClip);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
    }
	{
		// Drag a small clip on top of the clip left of the transition. This left clip
		// is made shorter, but the transition remains.
        PrepareSnapping(false);
		pixel right = RightPixel(VideoClip(0,1));
		right -= 20; // Ensure that 'a bit' of the clip left of the transition remains, causing the transition to remain also
        pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
        DragAlignRight(Center(VideoClip(0,6)),right);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDraggedClip);
		ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.defaultSize);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
		Undo();
	}
	{
		// Drag a small clip on top of the clip right of the transition. This right clip
		// is made shorter, but the transition remains.
		PrepareSnapping(false);
		pixel left = LeftPixel(VideoClip(0,3));
		left += 20; // Ensure that 'a bit' of the clip right of the transition remains, causing the transition to remain also
		pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
		DragAlignLeft(Center(VideoClip(0,6)),left);
		ASSERT_EQUALS(VideoClip(0,4)->getLength(),lengthOfDraggedClip);
		ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
		ASSERT(VideoClip(0,2)->isA<model::Transition>());
		ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.defaultSize);
		ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
		Undo();
    }
    {
        // Move the leftmost of the two clips adjacent to the transition: the transition must be removed
        DeselectAllClips();
        Drag(Center(VideoClip(0,1)),Center(VideoClip(0,4)));
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());
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
        Click(Center(VideoClip(0,1)));
        wxPoint from = LeftCenter(VideoClip(0,1));
        from.x += 10;
        wxPoint to = Center(VideoClip(0,6));
        Drag(from, to);
        Undo();
    }
    {
        // Drag and drop the clip onto (approx.) the same position. That scenario caused bugs:
        // clip is removed (during drag-and-drop). At the end of the drag-and-drop,
        // the transition is 'undone'. The undoing of the transition made assumptions
        // on availability of adjacent clips, which was invalid (clip has just been moved).
        wxPoint from = LeftCenter(VideoClip(0,1)) + wxPoint(10,0);
        wxPoint to = Center(VideoClip(0,6));
        Drag(from,to,false,true,false);
        Drag(to,from,false,false,true);
        Undo();
    }

    //  Turn on snapping again
    checkMenu(ID_SNAP_CLIPS, true);
    checkMenu(ID_SNAP_CURSOR, true);
}

void TestTimeline::testAdjacentTransitions()
{
    Type('=');
    Type('=');
    Type('=');
    {
        // Reduce size of second and third clip to be able to create transitions
        TrimRight(VideoClip(0,1),30,false);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        TrimLeft(VideoClip(0,3),30,false); // Note: the trim of clip 1 causes clip 2 to become clip 3 (clip 2 is empty space)
        ASSERT(!VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT(!VideoClip(0,4)->isA<model::EmptyClip>());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd())(VideoClip(0,1));
        ASSERT_LESS_THAN_ZERO(VideoClip(0,3)->getMinAdjustBegin())(VideoClip(0,2));
        // Make transitions between clips 2 and 3
        Move(RightCenter(VideoClip(0,1)));
        Type('c');
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        waitForIdle();
        Move(LeftCenter(VideoClip(0,4)));
        Type('c');
        ASSERT(VideoClip(0,4)->isA<model::Transition>());
        ASSERT(!VideoClip(0,5)->isA<model::EmptyClip>());
        DragAlignLeft(Center(VideoClip(0,5)),RightPixel(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        Scrub(LeftPixel(VideoTransition(0,2)) - 5, RightPixel(VideoTransition(0,3)) + 5);
        Play(LeftPixel(VideoTransition(0,2)) - 2, 1500); // -2: Also take some frames from the left clip
    }
    {
        // Drag a clip just on top of the right transition. This effectively removes
        // that right transition and (part) of the clip to its right. The clips to the left
        // of the removed transition must remain unaffected.
        pts cliplength = VideoClip(0,1)->getLength();
        pts transitionlength = VideoClip(0,2)->getLength();
        pts length = VideoClip(0,8)->getLength();
        DragAlignLeft(Center(VideoClip(0,8)),LeftPixel(VideoClip(0,3)));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),cliplength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),transitionlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),length);
        Undo();
    }
    {
        // Drag a clip just on top of the left transition. This effectively removes
        // that left transition and (part) of the clip to its left. The clips to the right
        // of the removed transition must remain unaffected.
        pts cliplength = VideoClip(0,4)->getLength();
        pts transitionlength = VideoClip(0,3)->getLength();
        pts length = VideoClip(0,8)->getLength();
        DragAlignRight(Center(VideoClip(0,8)) + wxPoint(5,0),RightPixel(VideoClip(0,2))-2);
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),length);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),transitionlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),cliplength);
        Undo();
    }
}

void TestTimeline::testUndo()
{
    LOG_DEBUG << "TEST_START";

    pts length = VideoClip(0,3)->getLength();
    Drag(Center(VideoClip(0,3)),wxPoint(2,Center(VideoClip(0,3)).y));
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),length);
    Undo();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),length);

    Type('=');  // Zoom in
    MakeInOutTransitionAfterClip preparation(1);
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());

    // Move clip 2: the transition must be removed
    DeselectAllClips();
    Drag(Center(VideoClip(0,1)), Center(VideoClip(0,4)));
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    Undo();
    ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 3: the transition must be removed and the fourth clip becomes the third one (clip+transition removed)
    model::IClipPtr afterclip = VideoClip(0,4);
    DeselectAllClips();
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)));
    ASSERT_EQUALS(afterclip,VideoClip(0,3));
    ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateAutoFolder>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    Undo();
}

void TestTimeline::testTransition()
{
    // This tests (for In, Out as well as In&Out transitions)
    // - when deleting a transition, the related clip's lengths are adjusted
    //   accordingly (so that it looks as if the transition is just removed,
    //   without affecting these clips. which in fact are changed).
    // - when selecting only the clips related to the transition but not the
    //   transition itselves, the transition is also taken along when doing
    //   a DND operation.
    // - playback of transition
    // - scrubbing over the transition
    // - Undoing

    // Zoom in once to avoid clicking in the middle of a clip which is then
    // seen (logically) as clip end due to the zooming
    Type('=');

    {
        MakeInOutTransitionAfterClip preparation(1);

        // Select and delete transition only. Then, the remaining clips
        // must have their original lengths restored.
        Click(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        Type(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();

        // Move clips around transition: the transition must be moved also
        DeselectAllClips();
        Click(Center(VideoClip(0,1)));
        Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,5)->isA<model::Transition>());
        ASSERT_MORE_THAN_ZERO(VideoTransition(0,5)->getRight());
        ASSERT_MORE_THAN_ZERO(VideoTransition(0,5)->getLeft());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,9)->getRightPts(),AudioClip(0,8)->getRightPts());

        // Scrub and play  the transition
        Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
        VideoTransition(0,5)->getPrev();
        ASSERT(VideoTransition(0,5)->getTrack());
        Play(LeftPixel(VideoTransition(0,5)) - 2, 1000); // -2: Also take some frames from the left clip

        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        Undo();
    }
    {
        MakeInTransitionAfterClip preparation(1);

        // Select and delete transition only. Then, the remaining clips
        // must have their original lengths restored.
        Click(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        Type(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();

        // Move clip related to transition: the transition must be moved also
        DeselectAllClips();
        Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,5)->isA<model::Transition>());
        ASSERT_MORE_THAN_ZERO(VideoTransition(0,5)->getRight());
        ASSERT_ZERO(VideoTransition(0,5)->getLeft());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());

        // Scrub and play the transition
        Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
        Play(LeftPixel(VideoTransition(0,5)) - 2, 1500); // -2: Also take some frames from the left clip

        // Undo until the two trimmed clips are present
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        Undo();
    }
    {
        MakeOutTransitionAfterClip preparation(1);

        // Select and delete transition only. Then, the remaining clips
        // must have their original lengths restored.
        Click(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->getSelected());
        Type(WXK_DELETE);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
        Undo();

        // Move clip related to transition: the transition must be moved also
        DeselectAllClips();
        Drag(Center(VideoClip(0,1)), Center(VideoClip(0,5)), true);
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,6)->isA<model::Transition>());
        ASSERT_ZERO(VideoTransition(0,6)->getRight());
        ASSERT_MORE_THAN_ZERO(VideoTransition(0,6)->getLeft());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());

        // Scrub and play the transition
        Scrub(LeftPixel(VideoTransition(0,6)) - 5, RightPixel(VideoTransition(0,6)) + 5);
        Play(LeftPixel(VideoTransition(0,6)) - 2, 1000); // -1: Also take some frames from the left clip

        // Undo until the two trimmed clips are present
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        Undo();
    }
}

// todo test createtransition when adjacent to empty clips.... all the generic transition creation used
// in the other tests uses a non-extendable transition instead of emptyness for the in, and out transitions.

void TestTimeline::testSplitting()
{
    MakeInOutTransitionAfterClip preparation(1);
    {
        PositionCursor(HCenter(VideoClip(0,2)));
        Type('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
    {
        PositionCursor(LeftPixel(VideoClip(0,2)));
        Type('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
    {
        PositionCursor(RightPixel(VideoClip(0,2)));
        Type('s');
        ASSERT(!VideoClip(0,0)->isA<model::Transition>());
        ASSERT(!VideoClip(0,1)->isA<model::Transition>());
        ASSERT(!VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!VideoClip(0,4)->isA<model::Transition>());
        Undo();
    }
}

void TestTimeline::testAbortDrag()
{
    for (int zoom = 0; zoom < 4; zoom++)
    {
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

        DeselectAllClips();
        Drag(Center(VideoClip(0,5)), Center(VideoClip(0,4)), false, true, false);
        ShiftDown();
        Move(Center(VideoClip(0,3)));
        Type(WXK_ESCAPE); // Abort the drop
        wxUIActionSimulator().MouseUp();
        ShiftUp();

        ASSERT_MORE_THAN_EQUALS(getTimeline().getZoom().pixelsToPts(LeftCenter(VideoClip(0,1)).x),VideoClip(0,1)->getLeftPts());

        Undo();
        Type('=');  // Zoom in and test again
    }
}
} // namespace