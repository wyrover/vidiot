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
#include "TrimClip.h"
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
    START_TEST;
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
}

void TestTimeline::testDnd()
{
    START_TEST;
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
    {
        // Test moving one clip partially on top of its original location (caused a recursion error in AClipEdit,
        // for expanding the replacement map).
        PrepareSnapping(true);
        pts length = VideoClip(0,3)->getLength();
        Drag(Center(VideoClip(0,3)), Center(VideoClip(0,3)) + wxPoint(20,0)); // Move the clip only a bit to the right
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),length );
        Undo();
    }
}

void TestTimeline::testUndo()
{
    START_TEST;

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
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    Undo(); ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateAutoFolder>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Redo(); ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    Undo();
}

void TestTimeline::testSplitting()
{
    START_TEST;
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
    START_TEST;
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