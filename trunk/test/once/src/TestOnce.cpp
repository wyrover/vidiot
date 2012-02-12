#pragma warning(disable:4996)
#include "TestOnce.h"

#include <wx/uiaction.h>
#include "typeinfo.h"

#include <boost/algorithm/string.hpp>
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

struct TypeAsserter
    :   boost::noncopyable
{
    TypeAsserter& TYPEASSERTER_A;   ///< Helper, in order to be able to compile the code (TYPEASSERTER_* macros)
    TypeAsserter& TYPEASSERTER_B;   ///< Helper, in order to be able to compile the code (TYPEASSERTER_* macros)

    TypeAsserter(bool video, int TrackNumber)
        :   TYPEASSERTER_A(*this)
        ,   TYPEASSERTER_B(*this)
        ,   mVideo(video)
        ,   mTrackNumber(TrackNumber)
        ,   mClipNumber(0)
    {
    }

    ~TypeAsserter()
    {
    };

    template<class type>
    TypeAsserter& AssertClipType()
    {
        bool ok = mVideo ? VideoClip(mTrackNumber,mClipNumber)->isA<type>() : AudioClip(mTrackNumber,mClipNumber)->isA<type>();
        if (!ok)
        {
            int TrackNumber = mTrackNumber;
            int ClipNumber = mClipNumber;
            std::string TrackType = mVideo ? "VIDEO" : "AUDIO";
            auto convert = [](const type_info& info) -> std::string
            {
                std::vector<std::string> strs;
                boost::split(strs, std::string(info.name()), boost::is_any_of(":"));
                return strs.back();
            };

            std::string Expected = convert(typeid(type));
            std::string Got = convert(typeid(*VideoClip(TrackNumber,mClipNumber)));
            LogVar("Clip type error", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A(TrackType)(TrackNumber)(mClipNumber)(Expected)(Got);
        }
        mClipNumber++;
        return *this;
    }

    int mTrackNumber;
    int mClipNumber;
    bool mVideo;
};

#define TYPEASSERTER_A(type) TYPEASSERTER_OP(type, B)
#define TYPEASSERTER_B(type) TYPEASSERTER_OP(type, A)
#define TYPEASSERTER_OP(type, next) TYPEASSERTER_A.AssertClipType<model::type>().TYPEASSERTER_ ## next
#define ASSERT_VIDEOTRACK(TrackNumber) TypeAsserter(true,TrackNumber).TYPEASSERTER_A
#define ASSERT_AUDIOCLIPTYPES TypeAsserter(false,TrackNumber).TYPEASSERTER_A

void TestOnce::testOnce()
{
    StartTestSuite();

    Zoom Level(4);

    {
        StartTest("In-out-transition: when trimming the linked audio clips the transition is also unapplied.");
        MakeInOutTransitionAfterClip preparation(1);
        pts originalLengthOfAudioClip1 = AudioClip(0,1)->getLength();
        pts originalLengthOfAudioClip2 = AudioClip(0,2)->getLength();

        ////////// TESTS WITHOUT SHIFT ////////
        //StartTest("In-out-transition: reduce the size of the clip linked to the out-clip (transition must be removed)");
        //Drag(RightCenter(AudioClip(0,1)),Center(AudioClip(0,1)));
        //ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        //ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        //ASSERT_LESS_THAN(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        //ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        //ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        //ASSERT_EQUALS(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        //ASSERT_EQUALS(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        //Undo();
        //StartTest("In-out-transition: reduce the size of the clip linked to the in-clip (transition must be removed)");
        //Drag(LeftCenter(AudioClip(0,2)),Center(AudioClip(0,2)));
        //ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        //ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        //ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        //ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        //ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        //ASSERT_LESS_THAN(VideoClip(0,3)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        //ASSERT_LESS_THAN(AudioClip(0,3)->getLength(),originalLengthOfAudioClip2);
        //Undo();
        //StartTest("In-out-transition: reduce the size of the clip linked to the out-clip  as much as possible (transition must be removed)");
        //Drag(RightCenter(AudioClip(0,1)),LeftCenter(AudioClip(0,0)));
        //ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        //ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        //ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());
        //ASSERT_EQUALS(VideoClip(0,2)->getLength(),preparation.lengthOfClipAfterTransitionBeforeTransitionApplied);
        //ASSERT_EQUALS(AudioClip(0,2)->getLength(),originalLengthOfAudioClip2);
        //Undo();
        //StartTest("In-out-transition: reduce the size of the clip linked to the in-clip as much as possible (transition between videos must be removed)");
        //pts lengthOfVideoClipAfterOutClip = VideoClip(0,4)->getLength();
        //pts lengthOfAudioClipLinkedToVideoClipAfterOutClip = AudioClip(0,3)->getLength();
        //Drag(LeftCenter(AudioClip(0,2)),RightCenter(AudioClip(0,3)));
        //ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
        //ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionBeforeTransitionApplied);
        //ASSERT_EQUALS(AudioClip(0,1)->getLength(),originalLengthOfAudioClip1);
        //ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        //ASSERT(AudioClip(0,2)->isA<model::EmptyClip>());
        //ASSERT_EQUALS(VideoClip(0,3)->getLength(),lengthOfVideoClipAfterOutClip);
        //ASSERT_EQUALS(AudioClip(0,3)->getLength(),lengthOfAudioClipLinkedToVideoClipAfterOutClip);
        //Undo();
        StartTest("In-out-transition: reduce the 'other' side of the clip linked to the in-clip as much as possible (transition is NOT removed)");
        Drag(LeftCenter(AudioClip(0,1)),RightCenter(AudioClip(0,2)));
//        ASSERT_VIDEO_TYPES<model::VideoClip, model::EmptyClip, model::VideoClip, model::Transition, model::VideoClip, model::Transition>();
        //ASSERT_VIDEOCLIPTYPES(model::VideoClip)(model::EmptyClip)(model::VideoClip)(model::Transition)(model::VideoClip)(model::Transition);
        ASSERT_VIDEOTRACK(0)(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip)(VideoClip);
        //ASSERT_VIDEOCLIPTYPES(model::VideoClip,0)(model::EmptyClip,1)(model::VideoClip,2)(model::Transition,3)(model::VideoClip,4)(model::VideoClip,5)(model::VideoClip,6);
        ASSERT(!VideoClip(0,0)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),preparation.lengthOfClipBeforeTransitionAfterTransitionApplied);
        ASSERT(!VideoClip(0,2)->isA<model::EmptyClip>());
        ASSERT_ZERO(VideoClip(0,2)->getLength());
        ASSERT(!VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        ASSERT(!AudioClip(0,0)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());
        ASSERT(!AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT_EQUALS(AudioClip(0,1)->getLength() + AudioClip(0,2)->getLength(), originalLengthOfAudioClip1);
        ASSERT(!AudioClip(0,3)->isA<model::EmptyClip>());
        StartTest("In-out-transition: test scrubbing with an 'in' clip which is fully obscured by the transition");
        Scrub(RightPixel(VideoClip(0,1))-5, HCenter(VideoClip(0,3))); // Bug once: The empty video clip that is 'just before' the transition asserted when doing a moveTo(0), since !(0<length) for a clip with length 0.
        StartTest("In-out-transition: test enlarging an 'in' clip which is fully obscured by the transition");
        Drag(LeftVBottomQuarter(VideoClip(0,3)),LeftCenter(VideoClip(0,1)));
        // todo enlarge the clip again by trimming (does not work yet!!!)
       pause();
       Undo();
        StartTest("In-out-transition: reduce the 'other' side of the clip linked to the out-clip as much as possible (transition is NOT removed)");
        Drag(RightCenter(AudioClip(0,2)),Center(AudioClip(0,0)));
        ASSERT(!VideoClip(0,0)->isA<model::EmptyClip>());
        ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(!VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT_ZERO(VideoClip(0,3)->getLength());
        ASSERT(VideoClip(0,4)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),preparation.lengthOfClipAfterTransitionAfterTransitionApplied);
        ASSERT(!AudioClip(0,0)->isA<model::EmptyClip>());
        ASSERT(!AudioClip(0,1)->isA<model::EmptyClip>());
        ASSERT(!AudioClip(0,2)->isA<model::EmptyClip>());
        ASSERT(AudioClip(0,3)->isA<model::EmptyClip>());
        ASSERT_EQUALS(AudioClip(0,2)->getLength() + AudioClip(0,3)->getLength(), originalLengthOfAudioClip2);
        StartTest("In-out-transition: test scrubbing with an 'out' clip which is fully obscured by the transition");
        Scrub(HCenter(VideoClip(0,2)), LeftPixel(VideoClip(0,4)) + 5); // Bug once: The empty video clip that is 'just before' the transition asserted when doing a moveTo(0), since !(0<length) for a clip with length 0.
        StartTest("In-out-transition: test enlarging an 'out' clip which is fully obscured by the transition");
        // todo enlarge the clip again by trimming (does not work yet)
pause();        Undo();
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