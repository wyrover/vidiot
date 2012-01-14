#include "TestOnce.h"

#include <wx/uiaction.h>

#include <wx/gdicmn.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "Dialog.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "MousePointer.h"
#include "IClip.h"
#include "PositionInfo.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "ViewMap.h"
#include "Window.h"
#include "Zoom.h"
#include "ids.h"

namespace test {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestOnce::setUp()
{
    mProjectFixture.init();
}

void TestOnce::tearDown()
{
    mProjectFixture.destroy();
}

auto PrepareSnapping = [](bool enableSnapping)
{
    checkMenu(ID_SNAP_CLIPS, enableSnapping);
    checkMenu(ID_SNAP_CURSOR, enableSnapping);
    DeselectAllClips();
};

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestOnce::testOnce()
{
    START_TEST;

        // Zoom in
    Type('=');
    Type('=');
    Type('=');
    Type('=');

    {
        // Test - for an in-out-transition- that clicking on TransitionLeftClipEnd
        // starts a trim operation, not with the transition but the clip left of the transition.
        // Also test that the minadjustbegin/maxadjustend values are honored

        PrepareSnapping(true);
        MakeInOutTransitionAfterClip preparation(1);

        //////// TESTS WITHOUT SHIFT ////////
        // Test reducing the clip size
        Drag(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        // Test the lower resize bound (which must be such that the entire clip can be trimmed away, since the transition is unapplied)
        Drag(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        // Test enlarging the clip size (which is not possible)
        Drag(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        //////// TESTS WITH SHIFT ////////
        // Test reducing the clip size
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        // Test the lower resize bound (which is such that of the left clip only the part under the transition remains)
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        // Test enlarging the clip size (which is possible when shift dragging)
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,2)));
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        // Test the upper resize bound (only useful for shift dragging)
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionOriginal - preparation.lengthOfTransition);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
    }
    {
        // Test - for an out-only-transition - that clicking on TransitionLeftClipEnd
        // starts a trim operation, not with the transition but the clip left of the transition.
        // Also test that the minadjustbegin/maxadjustend values are honored

        PrepareSnapping(true);
        MakeOutTransitionAfterClip preparation(1);

        //////// TESTS WITHOUT SHIFT ////////
        // Test reducing the clip size
        Drag(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        // Test the lower resize bound (which must be such that the entire clip can be trimmed away, since the transition is unapplied)
        Drag(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        Undo();
        // Test enlarging the clip size (which is not possible)
        Drag(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        //////// TESTS WITH SHIFT ////////
        // Test reducing the clip size
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),Center(VideoClip(0,1)));
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        // Test the lower resize bound (which is such that of the left clip only the part under the transition remains)
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),LeftCenter(VideoClip(0,0)));
        ASSERT(!VideoClip(0,1)->isA<model::Transition>()); // There should still be a clip with length 0
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),0); // The left clip itselves has length 0, only the part under the transition is used
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfTransition);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        // Test enlarging the clip size (which is not possible)
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,2)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        // NOT: Undo();
        // Test the upper resize bound (only useful for shift dragging)
        ShiftDrag(TransitionLeftClipEnd(VideoClip(0,2)),RightCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionOriginal - preparation.lengthOfTransition);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        Undo();
        pause(30000);
    }

    {
        // Test - for an in-out-transition- that clicking on TransitionRightClipBegin
        // starts a trim operation, not with the transition but the clip right of the transition.

        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    {
        // Test - for an out-only-transition - that clicking on TransitionRightClipBegin
        // starts a trim operation, not with the transition but the clip right of the transition.

        // Also test that the minadjustbegin/maxadjustend values are honored
    }

    {
        // Test - for an in-out-transition - that clicking on TransitionInterior selects the clip.
        // Test that dragging works!?
    }
    {
        // Test - for an in-only-transition - that clicking on TransitionInterior selects the clip.
        // Test that dragging works!?
    }
    {
        // Test - for an out-only-transition - that clicking on TransitionInterior selects the clip.
        // Test that dragging works!?
    }

    {
        // Test - for an in-out-transition - that clicking on TransitionBegin starts trimming the
        // selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    {
        // Test - for an in-only-transition - that clicking on TransitionBegin starts trimming the
        // selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    {
        // Test - for an out-only-transition - that clicking on TransitionBegin starts trimming the
        // selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }

    {
        // Test - for an in-out-transition - that clicking on TransitionEnd starts trimming the
        // selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    {
        // Test - for an in-only-transition - that clicking on TransitionEnd starts trimming the
        // selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    {
        // Test - for an out-only-transition - that clicking on TransitionEnd starts trimming the
        // selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }

    {
        // Test - for an out-only-transition adjacent to an in-only-transition - that clicking on
        // TransitionEnd starts trimming the selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }

    {
        // Test - for an in-only-transition adjacent to an out-only-transition - that clicking on
        // TransitionBegin starts trimming the selected transition.
        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    // todo test clicking + dragging + trimming transition begin + end + interior
    // todo test clicking outside tracks etc.
    // todo test clicking on dividers
     // todo test trimming a clip so far that none of it remains
    // todo test for A-transition-B, moving A on top of the 'end' of B, and dropping B on the 'begin' of A
    // todo bug: start application, make sequence, shift click clip four. all first four clips selected!
}
} // namespace