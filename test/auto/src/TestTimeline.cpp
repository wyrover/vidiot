#include "TestTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "Details.h"
#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperTestSuite.h"
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
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "Zoom.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTimeline::setUp()
{
    mProjectFixture.init();
}

void TestTimeline::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY(testSelection);
void TestTimeline::testSelection()
{
    StartTestSuite();
    const model::IClips& clips = getSequence()->getVideoTrack(0)->getClips();
    {
        StartTest("Start application, make sequence, shift click clip five. All first five clips selected!");
        DeselectAllClips();
        ASSERT_SELECTION_SIZE(0);
        ShiftDown();
        Click(Center(VideoClip(0,4)));
        ShiftUp();
        ASSERT_SELECTION_SIZE(5);
    }
    {
        StartTest("CTRL clicking all clips one by one");
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
        StartTest("SHIFT clicking the entire list");
        DeselectAllClips();
        ASSERT_SELECTION_SIZE(0);
        ShiftDown();
        Click(Center(clips.front()));
        Click(Center(clips.back()));
        ShiftUp();
        ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    }
    {
        StartTest("SHIFT clicking only the partial list");
        DeselectAllClips();
        ASSERT_SELECTION_SIZE(0);
        Click(Center(VideoClip(0,2)));
        ShiftDown();
        Click(Center(VideoClip(0,4)));
        ShiftUp();
        ASSERT_SELECTION_SIZE(3);
    }
    {
        StartTest("(de)selecting one clip with CTRL click");
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
        StartTest("Select the transition between two clips when shift selecting.");
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
        StartTest("Select an in-out-transition.");
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
        StartTest("Select an out-only-transition.");
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
        StartTest("Select an in-only-transition.");
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
    {
        StartTest("When deleting a clip, an in-only, and a out-only transition must be deleted also.");
    }
};

//RUNONLY(testDnd);
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
        ConfigFixture.SnapToCursor(true);
        pts length = VideoClip(0,3)->getLength();
        DragAlignLeft(Center(VideoClip(0,3)),1); // Move to a bit after the beginning of timeline, snaps to the cursor
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
        ConfigFixture.SnapToClips(true);
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

//RUNONLY(testUndo);
void TestTimeline::testUndo()
{
    StartTestSuite();
    ConfigFixture.SnapToClips(true);
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
    ConfigFixture.SnapToClips(true);
    Zoom Level(2);

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
        const pixel originalDividerPosition = getTimeline().getSequenceView().getPosition(VideoTrack(0)) - gui::Layout::TrackDividerHeight;
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

//RUNONLY(testTrimming);
void TestTimeline::testTrimming()
{
    StartTestSuite();
    // todo make trimming test that uses snapping also
    Zoom Level(2);
    DeleteClip(VideoClip(0,3));
    DeleteClip(VideoClip(0,1));
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
    {
        StartTest("Trim: Without Shift: Reduce clip size left.");
        TrimLeft(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_MORE_THAN(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_LESS_THAN(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        StartTest("Trim: Without Shift: Enlarge clip size left.");
        TrimLeft(VideoClip(0,2),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        Undo();
        Undo();
    }
    {
        StartTest("Trim: Without Shift: Reduce clip size right.");
        TrimRight(VideoClip(0,2),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_LESS_THAN(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_MORE_THAN(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        StartTest("Trim: Without Shift: Enlarge clip size right.");
        TrimRight(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        Undo();
        Undo();
    }
    Undo();
    Undo();
    {
        StartTest("Trim: Left: During the trim, the Trim detailspanel is visible.");
        wxString description = VideoClip(0,3)->getDescription();
        BeginTrim(LeftCenter(VideoClip(0,3)), false);
        MoveRight(10);
        gui::timeline::DetailsTrim* detailstrim = dynamic_cast<gui::timeline::DetailsTrim*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailstrim);
        StartTest("Trim: After the trim is done, the Clip detailspanel is visible.");
        EndTrim(false);
        gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailsclip);
        ASSERT_EQUALS(detailsclip->getClip()->getDescription(),description);
        Undo();
    }
    {
        StartTest("Trim: Right: During the trim, the Trim detailspanel is visible.");
        wxString description = VideoClip(0,3)->getDescription();
        BeginTrim(RightCenter(VideoClip(0,3)), false);
        MoveLeft(10);
        gui::timeline::DetailsTrim* detailstrim = dynamic_cast<gui::timeline::DetailsTrim*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailstrim);
        StartTest("Trim: After pressing escape, the Clip detailspanel is visible.");
        Type(WXK_ESCAPE);
        waitForIdle();
        gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailsclip);
        ASSERT_EQUALS(detailsclip->getClip()->getDescription(),description);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        wxUIActionSimulator().MouseUp();
    }
}

void TestTimeline::testTrimmingWithOtherTracks()
{
    StartTestSuite();
    Zoom level(2);
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);
    {
        StartTest("Trim: EndTrim: Enlarge the last clip in a track (there is no empty clip after it anymore)");
        TrimRight(VideoClip(0,3),-40,false);
        pts length = VideoClip(0,3)->getLength();
        DragAlignLeft(Center(VideoClip(0,3)),RightPixel(VideoClip(0,7)));
        TrimRight(VideoClip(0,7),20,false);
        ASSERT_MORE_THAN(VideoClip(0,7)->getLength(), length);
        Undo();
        Undo();
        Undo();
    }
    {
        StartTest("ShiftTrim: BeginTrim: Shorten: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TrimLeft(VideoClip(0,4),100);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),previouslength);
        previouslength = VideoClip(0,4)->getLength();
    }
    {
        StartTest("ShiftTrim: BeginTrim: Enlarge: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TrimLeft(VideoClip(0,4),-10);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(),previouslength);
    }
    {
        StartTest("ShiftTrim: EndTrim: Shorten: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TrimRight(VideoClip(0,4),-20);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),previouslength);
    }
    {
        StartTest("ShiftTrim: EndTrim: Enlarge: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TrimRight(VideoClip(0,4),10);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(),previouslength);
    }
    {
        StartTest("Make a clip in another track (preparation).");
        DragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
    }
    {
        StartTest("Move the clip in the other track slightly over the begin of the tested clip (preparation).");
        Drag(Center(VideoClip(0,4)),RightCenter(VideoClip(0,3)));
    }
    {
        StartTest("ShiftTrim: EndTrim: Shorten: with another track that is shorter than the trim position (this imposes a lower bound on the shift).");
        pts diff = VideoClip(0,4)->getRightPts() - VideoTrack(1)->getLength();
        pts track0len = VideoTrack(0)->getLength();
        TrimRight(VideoClip(0,4),-400);
        ASSERT_EQUALS(VideoClip(0,4)->getRightPts(),VideoTrack(1)->getLength());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),track0len - diff);
        Undo(); // Undo the trim
    }
    pts previouslength = VideoClip(0,4)->getLength();
    {
        StartTest("ShiftTrim: BeginTrim: Shorten: with another track that has a clip on the trim position (no trim possible).");
        TrimLeft(VideoClip(0,4),-100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("ShiftTrim: BeginTrim: Enlarge: with another track that has a clip on the trim position (no trim possible).");
        TrimLeft(VideoClip(0,4),+100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
    }
    {
        StartTest("Move the clip in the other track over the end of the tested clip (preparation).");
        Drag(Center(VideoClip(1,1)),wxPoint(RightPixel(VideoClip(0,4)),VCenter(VideoTrack(1))));
    }
    {
        StartTest("ShiftTrim: EndTrim: Shorten: with another track that has a clip on the trim position (no trim possible).");
        TrimRight(VideoClip(0,4),-100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("ShiftTrim: EndTrim: Enlarge: with another track that has a clip on the trim position (no trim possible).");
        TrimRight(VideoClip(0,4),100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
    }
    {
        StartTest("Ensure that there's a clip in another track before AND after AND 'inbetween' the clip under test (preparation).");
        Undo();
        Undo();
        DragToTrack(1,VideoClip(0,5),AudioClip(0,5));
        TrimRight(VideoClip(0,6),-40);
    }
    {
        StartTest("ShiftTrim: Put clips in other tracks 'around' trim points but not exactly ON the trim point so that trimming is possible (Preparation).");
        DragToTrack(1,VideoClip(0,6),AudioClip(0,6));
        Drag(Center(VideoClip(1,1)),Center(VideoClip(1,1))-wxPoint(8,0));
        Drag(Center(VideoClip(1,3)),Center(VideoClip(1,3))+wxPoint(8,0));
        ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        DragAlignLeft(Center(VideoClip(1,4)),LeftPixel(VideoClip(0,4))+20);
        ASSERT_VIDEOTRACK1(EmptyClip)                   (VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(       VideoClip      )(EmptyClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(       AudioClip      )(EmptyClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                   (AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
    }
    previouslength = VideoClip(0,4)->getLength();
    pts minbegin = VideoClip(0,4)->getMinAdjustBegin();
    pts maxbegin = VideoClip(0,4)->getMaxAdjustBegin();
    pts minend = VideoClip(0,4)->getMinAdjustEnd();
    pts maxend = VideoClip(0,4)->getMaxAdjustEnd();
    {
        StartTest("ShiftTrim: BeginTrim: Shorten: other track space imposes a trim restriction.");
        pts maxadjust = VideoClip(1,3)->getLeftPts() - VideoClip(0,4)->getLeftPts();
        TrimLeft(VideoClip(0,4),200);
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(), previouslength - maxbegin); // Can't trim to the max due to the restriction in the other track
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength - maxadjust);
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),VideoClip(1,3)->getLeftPts());
        Undo();
    }
    {
        StartTest("ShiftTrim: BeginTrim: Enlarge: other track space imposes NO trim restriction.");
        TrimLeft(VideoClip(0,4),-200);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength - minbegin); // Note: minbegin < 0
        Undo();
    }
    {
        StartTest("ShiftTrim: EndTrim: Shorten: other track space imposes a trim restriction.");
        pts minadjust = VideoClip(1,3)->getRightPts() - VideoClip(0,4)->getRightPts();
        TrimRight(VideoClip(0,4),-200);
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(), previouslength + minend); // Can't trim to the max due to the restriction in the other track
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength + minadjust ); // Note: minadjust < 0
        ASSERT_EQUALS(VideoClip(0,4)->getRightPts(),VideoClip(1,3)->getRightPts());
        Undo();
    }
    {
        StartTest("ShiftTrim: EndTrim: Enlarge: other track space imposes NO trim restriction.");
        TrimRight(VideoClip(0,4),200);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength + maxend);
        Undo();
    }
}

} // namespace