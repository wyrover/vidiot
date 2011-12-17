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

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestOnce::testOnce()
{
    // rename to testTouchingTransitions

    auto PrepareSnapping = [](bool enableSnapping)
    {
        checkMenu(ID_SNAP_CLIPS, enableSnapping);
        checkMenu(ID_SNAP_CURSOR, enableSnapping);
        DeselectAllClips();
    };

        // Zoom in
    Type('=');
    Type('=');
    Type('=');
    Type('=');

    {
        // Test - for an in-out-transition- that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
        MakeInOutTransitionAfterClip preparation(1);

        Drag(TransitionLeftClipInterior(VideoClip(0,2)),Center(VideoClip(0,4)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),AudioClip(0,4)->getLength()); // Transition is unapplied which causes the audio and video to have the same lengths again
        ASSERT_EQUALS(VideoClip(0,-1)->getRightPts(),AudioClip(0,-1)->getRightPts()); // Both tracks have the same length
        pause(10000);
        Undo();
    }
    {
        // Test - for an out-only-transition - that dragging when clicking on TransitionLeftClipInterior
        // starts a drag and drop operation, not with the transition but the clip left of the transition.
    }
    {
        // Test - for an in-out-transition - that dragging when clicking on TransitionRightClipInterior starts a
        // drag and drop operation, not with the transition but the clip right of the transition.
        MakeOutTransitionAfterClip preparation(1);
    }
    {
        // Test - for an in-only-transition- that clicking on TransitionRightClipInterior
        // selects the clip right of the transition.
        MakeInTransitionAfterClip preparation(1);

    }
    {
        // Test - for an in-only-transition - that dragging when clicking on TransitionRightClipInterior starts a
        // drag and drop operation, not with the transition but the clip right of the transition.
        MakeInTransitionAfterClip preparation(1);
    }
    {
        // Test - for an in-out-transition- that clicking on TransitionLeftClipEnd
        // starts a trim operation, not with the transition but the clip left of the transition.

        // Also test that the minadjustbegin/maxadjustend values are honored
    }
    {
        // Test - for an out-only-transition- that clicking on TransitionLeftClipInterior
        // selects the clip left of the transition.
        MakeOutTransitionAfterClip preparation(1);

    }
    {
        // Test - for an out-only-transition - that clicking on TransitionLeftClipEnd
        // starts a trim operation, not with the transition but the clip left of the transition.

        // Also test that the minadjustbegin/maxadjustend values are honored
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

    // todo test for A-transition-B, moving A on top of the 'end' of B, and dropping B on the 'begin' of A
}
} // namespace