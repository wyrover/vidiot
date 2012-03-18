#include "TestTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "Layout.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
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
    StartTestSuite();
    const model::IClips& clips = getSequence()->getVideoTrack(0)->getClips();
    {
        // Start application, make sequence, shift click clip five. All first five clips selected!
        DeselectAllClips();
        ASSERT_SELECTION_SIZE(0);
        ShiftDown();
        Click(Center(VideoClip(0,4)));
        ShiftUp();
        ASSERT_SELECTION_SIZE(5);
    }
    {
        // Test CTRL clicking all clips one by one
        DeselectAllClips();
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
    {
        // Test selecting an in-out-transition
        MakeInOutTransitionAfterClip preparation(1);
        DeselectAllClips();
        Click(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        Click(VTopQuarterLeft(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        Click(VTopQuarterRight(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
    }
    {
        // Test selecting an out-only-transition
        MakeOutTransitionAfterClip preparation(1);
        DeselectAllClips();
        Click(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        Click(VTopQuarterLeft(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        Click(VTopQuarterRight(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
    }
    {
        // Test selecting an in-only-transition
        MakeInTransitionAfterClip preparation(1);
        DeselectAllClips();
        Click(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        Click(VTopQuarterLeft(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
        Click(VTopQuarterRight(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        DeselectAllClips();
    }
}

void TestTimeline::testDeletion()
{
    StartTestSuite();
    {
        StartTest("When deleting without shift, a clip is replaced with emptyness.");
        pts len = VideoTrack(0)->getLength();
        int num = NumberOfVideoClipsInTrack(0);
        DeselectAllClips();
        Click(Center(VideoClip(0,1)));
        ControlDown();
        Click(Center(VideoClip(0,3)));
        ASSERT_SELECTION_SIZE(2);
        ControlUp();
        Type(WXK_DELETE);
        ASSERT_SELECTION_SIZE(0);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),len);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num);
        Click(Center(VideoClip(0,5)));
        ASSERT_SELECTION_SIZE(1);
        Type(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(EmptyClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip)(EmptyClip);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),len);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num);
        ASSERT_SELECTION_SIZE(0);
        Undo();
        ASSERT_SELECTION_SIZE(1);
        Undo();
        ASSERT_SELECTION_SIZE(2);
    }
    {
        StartTest("When deleting with shift, a clip is replaced with emptyness and then the emptyness is removed.");
        pts len = VideoTrack(0)->getLength();
        int num = NumberOfVideoClipsInTrack(0);
        DeselectAllClips();
        Click(Center(VideoClip(0,1)));
        ControlDown();
        Click(Center(VideoClip(0,3)));
        ControlUp();
        ShiftDown();
        ASSERT_SELECTION_SIZE(2);
        Type(WXK_DELETE);
        ShiftUp();
        ASSERT_SELECTION_SIZE(0);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_LESS_THAN(VideoTrack(0)->getLength(),len);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num-2);
        Click(Center(VideoClip(0,2)));
        ASSERT_SELECTION_SIZE(1);
        ShiftDown();
        Type(WXK_DELETE);
        ShiftUp();
        ASSERT_SELECTION_SIZE(0);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num-3);
        Undo();
        ASSERT_SELECTION_SIZE(1);
        Undo();
        ASSERT_SELECTION_SIZE(2);
    }
};

void TestTimeline::testDnd()
{
    StartTestSuite();
    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
    Type('=');  // Zoom in
    {
        StartTest("Move one clip around.");
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
        StartTest("Drop a clip with snapping enabled does not affect the clip to the right of the snapping point.");
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
        StartTest("Move one clip partially on top of its original location (caused a recursion error in AClipEdit, for expanding the replacement map).");
        PrepareSnapping(true);
        pts length = VideoClip(0,3)->getLength();
        Drag(Center(VideoClip(0,3)), Center(VideoClip(0,3)) + wxPoint(20,0)); // Move the clip only a bit to the right
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),length );
        Undo();
    }
        {
        StartTest("Move a clip beyond the track length.");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        Drag(Center(VideoClip(0,1)), wxPoint(RightPixel(VideoClip(0,6)) + VideoClip(0,1)->getLength(), VCenter(VideoClip(0,1)))); // + + VideoClip(0,1)->getLength(): Ensure that it's dropped after a bit of empty space
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,8)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,8)->getLink(),AudioClip(0,8));
        ASSERT_EQUALS(AudioClip(0,8)->getLink(),VideoClip(0,8));
    }
}

void TestTimeline::testUndo()
{
    StartTestSuite();

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
    StartTestSuite();
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
    StartTestSuite();
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

void TestTimeline::testIntervals()
{
    StartTestSuite();
    Zoom Level(2);
    PrepareSnapping(true);

    StartTest("Make an interval from left to right and click 'delete all marked intervals'");
    ToggleInterval(HCenter(VideoClip(0,1)), HCenter(VideoClip(0,2)));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    pts video1Adjustedlength = VideoClip(0,1)->getLength();
    pts video2Adjustedlength = VideoClip(0,2)->getLength();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval from right to left and click 'delete all marked intervals'");
    ToggleInterval(HCenter(VideoClip(0,2)), HCenter(VideoClip(0,1)));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), video1Adjustedlength);
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), video2Adjustedlength);
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval that completely deletes a clip");
    ToggleInterval(LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    Undo();
    Undo();
}

void TestTimeline::testDividers()
{
    StartTestSuite();
    const pixel fixedX = 100; // Fixed x position on timeline
    const pixel changeY = 20; // Number of pixels to move the divider
    const pixel moveToMiddleOfDivider = 2; // Click somewhere in the middle of a divider
    {
        StartTest("Move the audio/video divider down and up again.");
        const pixel originalDividerPosition = getSequence()->getDividerPosition();
        const pixel adjustedDividerPosition = originalDividerPosition + changeY;
        wxPoint original(fixedX, originalDividerPosition + moveToMiddleOfDivider);
        wxPoint adjusted(fixedX, adjustedDividerPosition + moveToMiddleOfDivider);
        Drag(original, adjusted);
        Move(wxPoint(fixedX, 10)); // Was a bug once: the mouse release did not 'release' the move operation, and thus this move back up caused the divider back to its original position.
        ASSERT_EQUALS(getSequence()->getDividerPosition(), adjustedDividerPosition);
        Drag(adjusted, original);
        ASSERT_EQUALS(getSequence()->getDividerPosition(), originalDividerPosition);
    }
    {
        StartTest("Move audio track divider up and down again.");
        const pixel originalHeight = AudioTrack(0)->getHeight();
        const pixel originalDividerPosition = getTimeline().getSequenceView().getPosition(AudioTrack(0)) + AudioTrack(0)->getHeight();
        const pixel adjustedDividerPosition = originalDividerPosition - changeY;
        wxPoint original(fixedX, originalDividerPosition + moveToMiddleOfDivider);
        wxPoint adjusted(fixedX, adjustedDividerPosition + moveToMiddleOfDivider);
        Drag(original, adjusted);
        Move(wxPoint(fixedX, 10)); // Was a bug once: the mouse release did not 'release' the move operation, and thus this move back up caused the divider back to its original position.
        ASSERT_EQUALS(AudioTrack(0)->getHeight(), originalHeight - changeY);
        Drag(adjusted, original);
        ASSERT_EQUALS(AudioTrack(0)->getHeight(), originalHeight);
    }
    {
        StartTest("Move video track divider down and up again.");
        const pixel originalHeight = VideoTrack(0)->getHeight();
        const pixel originalDividerPosition = getTimeline().getSequenceView().getPosition(VideoTrack(0)) - gui::Layout::sTrackDividerHeight;
        const pixel adjustedDividerPosition = originalDividerPosition + changeY;
        wxPoint original(fixedX, originalDividerPosition + moveToMiddleOfDivider);
        wxPoint adjusted(fixedX, adjustedDividerPosition + moveToMiddleOfDivider);
        Drag(original, adjusted);
        Move(wxPoint(fixedX, 10)); // Was a bug once: the mouse release did not 'release' the move operation, and thus this move back up caused the divider back to its original position.
        ASSERT_EQUALS(VideoTrack(0)->getHeight(), originalHeight - changeY);
        Drag(adjusted, original);
        ASSERT_EQUALS(VideoTrack(0)->getHeight(), originalHeight);
    }
}

} // namespace