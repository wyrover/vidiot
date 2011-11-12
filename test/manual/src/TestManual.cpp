#include "TestManual.h"

#include <wx/uiaction.h>

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

void TestManual::setUp()
{
    mProjectFixture.init();
}

void TestManual::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestManual::testManual()
{
    // rename to testShiftDrag

    // Zoom in
    Type('=');
    Type('=');
    Type('=');
    // Reduce size of second and third clip to be able to create transition
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd())(VideoClip(0,1));
    ASSERT_LESS_THAN_ZERO(VideoClip(0,2)->getMinAdjustBegin())(VideoClip(0,2));
    // Store original lengths of clips
    pts leftPositionOfClipBeforeTransitionBeforeApplyingTransition  = LeftPixel(VideoClip(0,1));
    pts leftPositionOfClipAfterTransitionBeforeApplyingTransition   = LeftPixel(VideoClip(0,2));
    pts defaultSize                                                 = gui::Config::ReadLong(gui::Config::sPathDefaultTransitionLength);
    pts lengthOfFirstClip                                           = VideoClip(0,0)->getLength();
    pts lengthOfClipBeforeTransitionBeforeApplyingTransition        = VideoClip(0,1)->getLength();
    pts lengthOfClipAfterTransitionBeforeApplyingTransition         = VideoClip(0,2)->getLength();
    // Make transition before clip 3
    PositionCursor(LeftPixel(VideoClip(0,2)));
    Move(LeftCenter(VideoClip(0,2)));
    Type('c');
    // Store update lengths after creating transition
    pixel leftPositionOfClipBeforeTransitionAfterTransitionApplied = LeftPixel(VideoClip(0,1));
    pixel leftPositionOfTransitionAfterTransitionApplied           = LeftPixel(VideoClip(0,2));
    pixel leftPositionOfClipAfterTransitionAfterTransitionApplied  = LeftPixel(VideoClip(0,3));
    pts lengthOfClipBeforeTransitionAfterTransitionApplied       = VideoClip(0,1)->getLength();
    pts lengthOfClipAfterTransitionAfterTransitionApplied        = VideoClip(0,3)->getLength();
    ASSERT(VideoClip(0,2)->isA<model::Transition>())(VideoClip(0,2));
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeApplyingTransition - defaultSize / 2);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeApplyingTransition - defaultSize / 2);

    // Test: Shift drag without snapping enabled, 
    //       transition and its adjacent clips are shifted backwards
    checkMenu(ID_SNAP_CLIPS, false);
    checkMenu(ID_SNAP_CURSOR, false);
    DeselectAllClips(); 
    Drag(Center(VideoClip(0,6)), Center(VideoClip(0,5)), false, true, false);
    ShiftDown();
    Drag(Center(VideoClip(0,5)), Center(VideoClip(0,3)), false, false, true);
    ShiftUp();
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,4)->isA<model::Transition>());
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), lengthOfClipBeforeTransitionAfterTransitionApplied);
    ASSERT_EQUALS(VideoClip(0,5)->getLength(), lengthOfClipAfterTransitionAfterTransitionApplied);
    Undo();
    
    // Test: Shift drag without snapping enabled (drop over clip AFTER transition)
    //       clip after transition is shifted backwards -> transition is removed 
    //       because the two 'transitioned clips are separated'.
    //       (clip in front of transition remains intact)
    DeselectAllClips(); 
    Drag(Center(VideoClip(0,6)), Center(VideoClip(0,5)), false, true, false);
    ShiftDown();
    Drag(Center(VideoClip(0,5)), Center(VideoClip(0,4)), false, false, true);
    ShiftUp();
    ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), lengthOfClipBeforeTransitionAfterTransitionApplied);
    ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
    ASSERT_EQUALS(VideoClip(0,4)->getLength(), lengthOfClipAfterTransitionBeforeApplyingTransition);
    Undo();

    // Test: Shift drag with snapping enabled. The drop is done such that the left 
    //       position of the drop is aligned with the left position of the clip left
    //       of the transition
    pts lengthOfDraggedClip = VideoClip(0,6)->getLength();
    checkMenu(ID_SNAP_CLIPS, true);
    checkMenu(ID_SNAP_CURSOR, true);
    DeselectAllClips(); 
    ShiftDragAlignLeft(Center(VideoClip(0,6)),leftPositionOfClipBeforeTransitionAfterTransitionApplied);
    ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),lengthOfFirstClip);
    ASSERT_EQUALS(VideoClip(0,1)->getLength(),lengthOfDraggedClip);
    ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfClipBeforeTransitionAfterTransitionApplied);
    ASSERT(VideoClip(0,3)->isA<model::Transition>());
    Undo();
    
    // Test: Shift drag with snapping enabled. The drop is done such that the left 
    //       position of the drop is aligned with the left position of the transition.
    //       This causes the clip left of the transition to be shifted back.
    checkMenu(ID_SNAP_CLIPS, true);
    checkMenu(ID_SNAP_CURSOR, true);
    DeselectAllClips();
    pts lengthOfDraggedClip2 = VideoClip(0,5)->getLength(); // todo duplicate name, or store original values somewhere?
    ShiftDragAlignLeft(Center(VideoClip(0,5)),leftPositionOfTransitionAfterTransitionApplied);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),lengthOfFirstClip);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT_EQUALS(VideoClip(0,2)->getLength(),lengthOfDraggedClip2);
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),lengthOfClipBeforeTransitionAfterTransitionApplied);
    ASSERT(VideoClip(0,4)->isA<model::Transition>());
    ASSERT_EQUALS(VideoClip(0,5)->getLength(),lengthOfClipAfterTransitionAfterTransitionApplied);
    Undo();

    //ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK();  // todo dit kan alleen waar worden als gesnapped wordt op center van transition

    // Test 5: Shift drag with snapping enabled. The drop is done such that the left 
    //         position of the drop is aligned with the left position of the clip after
    //    the transition.


    // Test 6: Drag a small clip on top of the clip left of the transition. This left clip
    //         is made shorter, but the transition remains.

    // Test 7: Drag a small clip on top of the clip right of the transition. This right clip
    //         is made shorter, but the transition remains.

    pause(3000);
    DragAlignLeft(leftPositionOfTransitionAfterTransitionApplied);
    //pause(3000);
    DragAlignLeft(leftPositionOfClipAfterTransitionAfterTransitionApplied);
    pause();

    //  Turn on snapping again
    checkMenu(ID_SNAP_CLIPS, true);
    checkMenu(ID_SNAP_CURSOR, true);

}

} // namespace