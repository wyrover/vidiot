#include "TestTransition.h"

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

void TestTransition::setUp()
{
    mProjectFixture.init();
    PrepareSnapping(true);
}

void TestTransition::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTransition::testSelection()
{
    START_TEST;
    // Zoom in (required for correct positioning)
    Type('=');
    Type('=');
    Type('=');
    Type('=');
    {
        DeselectAllClips();
        MakeInOutTransitionAfterClip preparation(1);
        // Test - for an in-out-transition- that clicking on TransitionLeftClipInterior selects the clip left of the transition.
        Click(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        // Test - for an in-out-transition- that clicking on TransitionRightClipInterior selects the clip right of the transition.
        Click(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        // Test - for an in-out-transition- that clicking on TransitionLeftClipEnd selects the clip left of the transition.
        Click(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        // Test - for an in-out-transition- that clicking on TransitionRightClipBegin selects the clip right of the transition.
        Click(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
    }
    {
        DeselectAllClips();
        MakeOutTransitionAfterClip preparation(1);
        // Test - for an out-transition - that clicking on TransitionLeftClipInterior selects the clip left of the transition.
        Click(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        // Test - for an out-transition - that clicking on TransitionLeftClipEnd selects the clip left of the transition.
        Click(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
    }
    {
        DeselectAllClips();
        MakeInTransitionAfterClip preparation(1);
        // Test - for an in-transition- that clicking on TransitionRightClipInterior selects the clip right of the transition.
        Click(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        DeselectAllClips();
        // Test - for an in-transition- that clicking on TransitionRightClipBegin selects the clip right of the transition.
        Click(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
    }
}

void TestTransition::testDragAndDrop()
{
    START_TEST;
    // Zoom in
    Type('=');
    Type('=');
    Type('=');
    MakeInOutTransitionAfterClip preparation(1);
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,2));

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

void TestTransition::testInOutTransitionDragAndDrop()
{
    // Zoom in (required for correct positioning)
    Type('=');
    Type('=');
    Type('=');
    Type('=');
    {
        PrepareSnapping(true);
        // Test - for an in-out-transition- that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
        MakeInOutTransitionAfterClip preparation(1);

        ShiftDragAlignLeft(TransitionLeftClipInterior(VideoClip(0,2)),LeftPixel(VideoClip(0,4)));
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
        ShiftDragAlignLeft(TransitionRightClipInterior(VideoClip(0,2)),LeftPixel(VideoClip(0,4)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),AudioClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
}

void TestTransition::testInTransitionDragAndDrop()
{
    // Zoom in (required for correct positioning)
    Type('=');
    Type('=');
    Type('=');
    Type('=');
    {
        // Test - for an in-only-transition - that dragging when clicking on TransitionRightClipInterior starts a
        // drag and drop operation, not with the transition but the clip right of the transition.
        // The transition in this case is dragged along with the clip, since the only clip related to the
        // transition is the clip being dragged.
        MakeInTransitionAfterClip preparation(1);
        ShiftDragAlignLeft(TransitionRightClipInterior(VideoClip(0,2)),LeftPixel(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength() + VideoClip(0,4)->getLength(),AudioClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
}

void TestTransition::testOutTransitionDragAndDrop()
{
    // Zoom in (required for correct positioning)
    Type('=');
    Type('=');
    Type('=');
    Type('=');
    {
        // Test - for an out-only-transition - that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
        // The transition in this case is dragged along with the clip, since the only clip related to the
        // transition is the clip being dragged.
        MakeOutTransitionAfterClip preparation(1);

        ASSERT_EQUALS(VideoClip(0,1)->getLength() + VideoClip(0,2)->getLength(),AudioClip(0,1)->getLength()); // Transition is unapplied which causes the audio and video to have the same lengths again
        ShiftDragAlignLeft(TransitionLeftClipInterior(VideoClip(0,2)),LeftPixel(VideoClip(0,4)));
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied); // Clip and transition are replaced with emptyness
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,4)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength() + VideoClip(0,4)->getLength(),AudioClip(0,3)->getLength());
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        Undo();
    }
}

void TestTransition::testAdjacentTransitions()
{
    START_TEST;
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

void TestTransition::testPlaybackAndScrubbing()
{
    START_TEST;

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

} // namespace