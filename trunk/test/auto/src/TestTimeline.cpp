#include "TestTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
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
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoTransition.h"

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
        Click(Center(clip));
    }
    ControlUp();
    ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    DeselectAllClips();
    ASSERT_SELECTION_SIZE(0);

    // Test SHIFT clicking the entire list
    ShiftDown();
    Click(Center(clips.front()));
    Click(Center(clips.back()));
    ShiftUp();
    ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());

    // Test SHIFT clicking only the partial list
    DeselectAllClips();
    ASSERT_SELECTION_SIZE(0);
    Click(Center(VideoClip(0,2)));
    ShiftDown();
    Click(Center(VideoClip(0,4)));
    ShiftUp();
    ASSERT_SELECTION_SIZE(3);

    // Test (de)selecting one clip with CTRL click
    ControlDown();
    Click(Center(VideoClip(0,3)));
    ControlUp();
    ASSERT_SELECTION_SIZE(2);
    ControlDown();
    Click(Center(VideoClip(0,3)));
    ControlUp();
    ASSERT_SELECTION_SIZE(3);

    // Test selection the transition between two clips when shift selecting
    DeselectAllClips();
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Type('c');
    Click(Center(VideoClip(0,1)));
    ShiftDown();
    Click(Center(VideoClip(0,3)));
    ShiftUp();
    ASSERT(VideoClip(0,2)->isA<model::Transition>() && VideoClip(0,2)->getSelected());
}

void TestTimeline::testDnd()
{
    LOG_DEBUG << "TEST_START";

    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));

    // Test moving one clip around
    wxPoint from = Center(VideoClip(0,3));
    wxPoint to = from;
    to.x = 2; // Move to the beginning of timeline
    pts length = VideoClip(0,3)->getLength();
    Drag(from,to);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),length);
    Undo();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),length );
    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLink(),AudioClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));

    Type('=');  // Zoom in

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Type('c');
    waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 2: the transition must be removed
    getTimeline().getSelection().unselectAll();
    Drag(Center(VideoClip(0,1)),Center(VideoClip(0,4)));
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    ASSERT_EQUALS(VideoClip(0,0)->getLink(),AudioClip(0,0));
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(AudioClip(0,1)->isA<model::EmptyClip>());
    ASSERT_EQUALS(VideoClip(0,2)->getLink(),AudioClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLink(),AudioClip(0,3));
    ASSERT_EQUALS(VideoClip(0,4)->getLink(),AudioClip(0,4));
}

void TestTimeline::testUndo()
{
    LOG_DEBUG << "TEST_START";

    pts length = VideoClip(0,3)->getLength();
    Drag(Center(VideoClip(0,3)),wxPoint(2,Center(VideoClip(0,3)).y));
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),length);
    Undo();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),length);

    Type('=');  // Zoom in

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    Type('c');
    waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

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

    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();

    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateAutoFolder>();

    Redo();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    Redo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    Redo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    Redo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();

    Redo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
}

void TestTimeline::testTransition()
{
    LOG_DEBUG << "TEST_START";

    // This tests (for In, Out as well as In&Out transitions)
    // - when deleting a transition, the related clip's lengths are adjusted 
    //   accordingly (so that it looks as if the transition is just removed, 
    //   without affecting these clips. which in fact are changed).
    // - when selecting only the clips related to the transition but not the
    //   transition itselves, the transition is also taken along when doing
    //   a DND operation.
    // - playback of transition
    // - scrubbing over the transition
    // - Undoing 

    // Zoom in once to avoid clicking in the middle of a clip which is then 
    // seen (logically) as clip end due to the zooming
    Type('=');

    //////////////////////////////////////////////////////////////////////////
    // Transition between two clips

    // Make transition before clip 3
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd())(VideoClip(0,1));
    ASSERT_LESS_THAN_ZERO(VideoClip(0,2)->getMinAdjustBegin())(VideoClip(0,2));
    pts l1 = VideoClip(0,1)->getLength(); // Store for checking later on
    pts l2 = VideoClip(0,2)->getLength(); // Store for checking later on
    PositionCursor(LeftPixel(VideoClip(0,2)));
    Move(LeftCenter(VideoClip(0,2)));
    Type('c');
    ASSERT(VideoClip(0,2)->isA<model::Transition>())(VideoClip(0,2));
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,2)->getRight());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,2)->getLeft());

    // Select and delete transition only. Then, the remaining clips 
    // must have their original lengths restored.
    Click(VQuarterHCenter(VideoClip(0,2)));
    ASSERT(VideoClip(0,2)->getSelected());
    Type(WXK_DELETE);
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), l1); 
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), l2); 
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
    Undo();

    // Move clips around transition: the transition must be moved also
    DeselectAllClips();
    Click(Center(VideoClip(0,1)));
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,5)->getRight());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,5)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,9)->getRightPts(),AudioClip(0,8)->getRightPts());

    // Scrub and play  the transition
    Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
    VideoTransition(0,5)->getPrev();
    ASSERT(VideoTransition(0,5)->getTrack());
    Play(LeftPixel(VideoTransition(0,5)) - 2, 1000); // -2: Also take some frames from the left clip

    // Undo until the two trimmed clips are present
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>(); 
    Undo(); 
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    //////////////////////////////////////////////////////////////////////////
    // "In" Transition 

    // Delete leftmost clip (clip 2)
    DeselectAllClips();
    Click(Center(VideoClip(0,1)));
    Type(WXK_DELETE);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());

    // Make transition before clip 3
    Move(LeftCenter(VideoClip(0,2)));
    Type('c');
    ASSERT(VideoClip(0,2)->isA<model::Transition>());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,2)->getRight());
    ASSERT_ZERO(VideoTransition(0,2)->getLeft());

    // Select and delete transition only. Then, the remaining clips 
    // must have their original lengths restored.
    Click(VQuarterHCenter(VideoClip(0,2)));
    ASSERT(VideoClip(0,2)->getSelected());
    Type(WXK_DELETE);
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), l1); 
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), l2); 
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
    Undo();

    // Move clip related to transition: the transition must be moved also
    DeselectAllClips();
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,4)->isA<model::Transition>());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,4)->getRight());
    ASSERT_ZERO(VideoTransition(0,4)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());

    // Scrub and play the transition
    Scrub(LeftPixel(VideoTransition(0,4)) - 5, RightPixel(VideoTransition(0,4)) + 5);
    Play(LeftPixel(VideoTransition(0,4)) - 2, 1500); // -2: Also take some frames from the left clip

    // Undo until the two trimmed clips are present
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();

    //////////////////////////////////////////////////////////////////////////
    // "Out" Transition 

    // Delete rightmost clip (clip 3)
    DeselectAllClips();
    Click(Center(VideoClip(0,2)));
    Type(WXK_DELETE);
    ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());

    // Make transition before clip 3
    PositionCursor(RightPixel(VideoClip(0,1)));
    Move(RightCenter(VideoClip(0,1)));
    Type('c');
    DumpTimeline();
    VAR_DEBUG(RightCenter(VideoClip(0,1)));
    ASSERT(VideoClip(0,2)->isA<model::Transition>());
    ASSERT_ZERO(VideoTransition(0,2)->getRight());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,2)->getLeft());

    // Select and delete transition only. Then, the remaining clips 
    // must have their original lengths restored.
    Click(VQuarterHCenter(VideoClip(0,2)));
    ASSERT(VideoClip(0,2)->getSelected());
    Type(WXK_DELETE);
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), l1); 
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), l2); 
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
    Undo();

    // Move clip related to transition: the transition must be moved also
    DeselectAllClips();
    Drag(Center(VideoClip(0,1)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT_ZERO(VideoTransition(0,5)->getRight());
    ASSERT_MORE_THAN_ZERO(VideoTransition(0,5)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());

    // Scrub and play the transition
    Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
    Play(LeftPixel(VideoTransition(0,5)) - 2, 1000); // -1: Also take some frames from the left clip

    // Undo until the two trimmed clips are present
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();
}

void TestTimeline::testSplitting()
{
    // rename to testSplitting

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Type('c');
    waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    PositionCursor(HCenter(VideoClip(0,2)));
    Type('s');
    ASSERT(!VideoClip(0,0)->isA<model::Transition>());
    ASSERT(!VideoClip(0,1)->isA<model::Transition>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    ASSERT(!VideoClip(0,3)->isA<model::Transition>());
    ASSERT(!VideoClip(0,4)->isA<model::Transition>());
}
} // namespace