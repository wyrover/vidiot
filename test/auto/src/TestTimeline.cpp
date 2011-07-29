#include "TestTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "CreateTransition.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "VideoClip.h"

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

void TestTimeline::testSelection()
{
    LOG_DEBUG << "TEST_START";

    const model::IClips& clips = getSequence()->getVideoTrack(0)->getClips();

    int nClips = NumberOfVideoClipsInTrack(0);

    // Test CTRL clicking all clips one by one
    ControlDown();
    BOOST_FOREACH(model::IClipPtr clip, clips)
    {
        Click(clip);
    }
    ControlUp();
    ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    DeselectAllClips();
    ASSERT_SELECTION_SIZE(0);

    // Test SHIFT clicking the entire list
    ShiftDown();
    Click(clips.front());
    Click(clips.back());
    ShiftUp();
    ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());

    // Test SHIFT clicking only the partial list
    DeselectAllClips();
    ASSERT_SELECTION_SIZE(0);
    Click(VideoClip(0,2));
    ShiftDown();
    Click(VideoClip(0,4));
    ShiftUp();
    ASSERT_SELECTION_SIZE(3);

    // Test (de)selecting one clip with CTRL click
    ControlDown();
    Click(VideoClip(0,3));
    ControlUp();
    ASSERT_SELECTION_SIZE(2);
    ControlDown();
    Click(VideoClip(0,3));
    ControlUp();
    ASSERT_SELECTION_SIZE(3);

    // Test selection the transition between two clips when shift selecting
    DeselectAllClips();
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    wxUIActionSimulator().Char('c');
    waitForIdle();
    Click(VideoClip(0,1));
    ShiftDown();
    Click(VideoClip(0,3));
    ShiftUp();
    ASSERT(VideoClip(0,2)->isA<model::Transition>() && VideoClip(0,2)->getSelected());
}

void TestTimeline::testTransition()
{
    LOG_DEBUG << "TEST_START";

    // Give focus to timeline
    wxUIActionSimulator().MouseMove(TimelinePosition() + wxPoint(LeftPixel(VideoClip(0,2)), TopPixel(VideoClip(0,2))));
    wxUIActionSimulator().MouseClick();

    // Zoom in maximally. This is required to have accurate pointer positioning further on.
    // Without this, truncating integers in ptsToPixels and pixelsToPts causes wrong pointer placement.
    ControlDown();
    wxUIActionSimulator().Char('=');
    ControlUp();

    // Shift Trim clips to make room for transition
    TrimLeft(VideoClip(0,2),50,true);
    TrimRight(VideoClip(0,1),50,true);

    // Determine length of second and third clips
    pts secondClipLength = VideoClip(0,1)->getLength();
    pts thirdClipLength = VideoClip(0,2)->getLength();

    // Create crossfade
    ASSERT(getNonEmptyClipsCount() == mProjectFixture.InputFiles.size() * 2 );
    wxUIActionSimulator().Char('c');
    waitForIdle();
    ASSERT(getNonEmptyClipsCount() == mProjectFixture.InputFiles.size() * 2 + 1); // Transition added
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(VideoClip(0,2));
    ASSERT(transition);
    ASSERT(transition->getLeft() > 0)(transition);
    ASSERT(transition->getRight() > 0)(transition);

    pts secondClipLengthWithTransition = VideoClip(0,1)->getLength(); // Counting is 0-based, 1 -> clip 2
    pts thirdClipLengthWithTransition  = VideoClip(0,3)->getLength(); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip after the crossfade
    Click(VideoClip(0,3)); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)
    ASSERT_SELECTION_SIZE(1);
    wxUIActionSimulator().KeyDown(WXK_DELETE);
    wxUIActionSimulator().KeyUp(WXK_DELETE);
    waitForIdle();
    ASSERT(getNonEmptyClipsCount() == mProjectFixture.InputFiles.size() * 2 - 2); // Clip and link and transition removed
    ASSERT(secondClipLength == VideoClip(0,1)->getLength()); // Original length of second clip must be restored

    triggerUndo(); // Trigger undo of delete
    ASSERT(secondClipLengthWithTransition == VideoClip(0,1)->getLength());
    ASSERT(thirdClipLengthWithTransition  == VideoClip(0,3)->getLength()); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip before the crossfade
    Click(VideoClip(0,1));
    ASSERT_SELECTION_SIZE(1);
    wxUIActionSimulator().KeyDown(WXK_DELETE);
    wxUIActionSimulator().KeyUp(WXK_DELETE);
    waitForIdle();
    ASSERT(getNonEmptyClipsCount() == mProjectFixture.InputFiles.size() * 2 - 2); // Clip and link and transition removed
    ASSERT(thirdClipLength == VideoClip(0,2)->getLength()); // Original length of third clip must be restored

    triggerUndo(); // Trigger undo of delete
    ASSERT(secondClipLengthWithTransition == VideoClip(0,1)->getLength());
    ASSERT(thirdClipLengthWithTransition  == VideoClip(0,3)->getLength()); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    pixel top = TopPixel(VideoClip(0,2)) - 5;
    pixel left = LeftPixel(VideoClip(0,2)) - 1;
    pixel right = RightPixel(VideoClip(0,2)) + 1;
    wxUIActionSimulator().MouseMove(TimelinePosition() + wxPoint(left,top));
    wxUIActionSimulator().MouseDown();
    for (int i = left; i < right; ++i)
    {
        wxUIActionSimulator().MouseMove(TimelinePosition() + wxPoint(i,top));
        waitForIdle();
    }
    wxUIActionSimulator().MouseUp();
}

void TestTimeline::testDnd()
{
    LOG_DEBUG << "TEST_START";

    // Test moving one clip around
    wxPoint from = Center(VideoClip(0,3));
    wxPoint to = from;
    to.x = 2; // Move to the beginning of timeline

    pts length = VideoClip(0,3)->getLength();

    wxUIActionSimulator().MouseMove(TimelinePosition() + Center(VideoClip(0,3)));
    wxUIActionSimulator().MouseDown();
    waitForIdle();

    for (int i = 1; i < 10; ++i)
    {
        wxPoint p(from.x + (from.x - to.x) / i, from.y + (from.y - to.y) / i); 
        wxUIActionSimulator().MouseMove(TimelinePosition() + p);
        waitForIdle();
    }
    wxUIActionSimulator().MouseUp();
    waitForIdle();

    ASSERT( VideoClip(0,0)->getLength() == length );
    triggerUndo();
    ASSERT( VideoClip(0,3)->getLength() == length );

    // Zoom in
    wxUIActionSimulator().Char('=');
    waitForIdle();

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    wxUIActionSimulator().Char('c');
    waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 2: the transition must be removed
    getTimeline().getSelection().unselectAll();
    from = Center(VideoClip(0,1));
    to = Center(VideoClip(0,4));
    wxUIActionSimulator().MouseMove(TimelinePosition() + from);
    wxUIActionSimulator().MouseDown();
    for (int i = 1; i < 25; ++i)
    {
        from.x += 5;
        wxPoint p(from.x + (from.x - to.x) / i, from.y + (from.y - to.y) / i); 
        wxUIActionSimulator().MouseMove(TimelinePosition() + p);
        waitForIdle();
    }

    wxUIActionSimulator().MouseUp();
    waitForIdle();
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
}

void TestTimeline::testUndo()
{
    LOG_DEBUG << "TEST_START";

    // The undo scenario at end was difficult to fix. It was caused by using Timeline as a 
    // identifying member for AClipEdit commands. Since the undo included undo'ing the creation
    // of the timeline, the timeline was no longer a good identifier. Therefore, these commands
    // now contain SequencePtr as identifier.
    //
    // This test also tests that when only one of the clips in a transition is moved, the 
    // transition is removed after dropping that clip.

    // Test moving one clip around
    wxPoint from = Center(VideoClip(0,3));
    wxPoint to(2,from.y); // Move to the beginning of timeline
    pts length = VideoClip(0,3)->getLength();
    Drag(from,to);
    ASSERT( VideoClip(0,0)->getLength() == length );
    triggerUndo();
    ASSERT( VideoClip(0,3)->getLength() == length );

    // Zoom in
    wxUIActionSimulator().Char('=');
    waitForIdle();

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    wxUIActionSimulator().Char('c');
    waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 2: the transition must be removed
    DeselectAllClips();
    Drag(Center(VideoClip(0,1)), Center(VideoClip(0,4)));
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());

    triggerUndo();
    ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 3: the transition must be removed and the fourth clip becomes the third one (clip+transition removed)
    model::IClipPtr afterclip = VideoClip(0,4);
    DeselectAllClips();
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)));
    ASSERT(afterclip == VideoClip(0,3));
    ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());

    triggerUndo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();

    triggerUndo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    triggerUndo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    triggerUndo();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    triggerUndo();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateAutoFolder>();

    triggerRedo();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    triggerRedo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    triggerRedo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    triggerRedo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();

    triggerRedo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
}


} // namespace