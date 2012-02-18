#include "TestOnce.h"

#include <wx/uiaction.h>
#include "typeinfo.h"

//#include <boost/algorithm/string.hpp>
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
#include "AudioClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
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
#include "VideoClip.h"
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
    StartTestSuite();

    Zoom Level(4);

    {
        StartTest("In-out-transition: when trimming the linked audio clips the transition is also unapplied.");
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
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip); // todo apply this assert in all tests
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
        // todo make ASSERT_LENGTH(model::ICLipPtr) instead of the line above
        Undo();
        Undo();
        // NOT: Undo(); The clip has been reduced and enlarged again
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