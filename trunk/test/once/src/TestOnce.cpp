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

    Zoom Level(4);

    {
        // Test - for an in-out-transition - that when trimming the linked audio clips,
        // the transition is also unapplied.
        MakeInOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();

        //////// TESTS WITHOUT SHIFT ////////
        // Test reducing the linked audio clip size (transition between videos must be removed)
        Drag(RightCenter(AudioClip(0,1)),Center(AudioClip(0,1)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        // Test reducing the linked audio clip size (transition between videos must be removed)
        Drag(LeftCenter(AudioClip(0,2)),Center(AudioClip(0,2)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        // Test reducing the linked audio clip size as much as possible (transition between videos must be removed)
        Drag(RightCenter(AudioClip(0,1)),LeftCenter(AudioClip(0,0)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());

        assert that clip is removed and that clip 4 after it becomes 3 etc.
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();
        // Test reducing the linked audio clip size as much as possible (transition between videos must be removed)
        Drag(LeftCenter(AudioClip(0,2)),Center(AudioClip(0,2)));
        ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT_LESS_THAN(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        Undo();

        // Test reducing the 'other' side of the linked audio clip size as much as possible
        Drag(LeftCenter(AudioClip(0,1)),Center(AudioClip(0,4)));
        Undo();
        // Test reducing the 'other' side of the linked audio clip size as much as possible
        Drag(RightCenter(AudioClip(0,2)),Center(AudioClip(0,0)));
        Undo();
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
    // todo test clicking outside tracks etc.
    // todo test clicking on dividers
     // todo test trimming a clip so far that none of it remains
    // todo test for A-transition-B, moving A on top of the 'end' of B, and dropping B on the 'begin' of A
    // todo bug: start application, make sequence, shift click clip four. all first four clips selected!
}
} // namespace