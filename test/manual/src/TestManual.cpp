#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
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
    LOG_DEBUG << "TEST_START";

    // Zoom in once to avoid clicking in the middle of a clip which is then 
    // seen (logically) as clip end due to the zooming
    Type('=');

    // This test ensures that when moving one or two clips around without
    // selecting the releated transition, the transition is also dragged.

    //////////////////////////////////////////////////////////////////////////
    // Transition between two clips

    // Make transition before clip 3
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    ASSERT(VideoClip(0,1)->getMaxAdjustEnd() > 0)(VideoClip(0,1));
    ASSERT(VideoClip(0,2)->getMinAdjustBegin() < 0)(VideoClip(0,2));
    PositionCursor(LeftPixel(VideoClip(0,2)));
    Move(LeftCenter(VideoClip(0,2)));
    Type('c');
    ASSERT(VideoClip(0,2)->isA<model::Transition>())(VideoClip(0,2));
    ASSERT(VideoTransition(0,2)->getRight());
    ASSERT(VideoTransition(0,2)->getLeft());

    // Move clips around transition: the transition must be moved also
    DeselectAllClips();
    Click(Center(VideoClip(0,1)));
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT(VideoTransition(0,5)->getRight());
    ASSERT(VideoTransition(0,5)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,9)->getRightPts(),AudioClip(0,8)->getRightPts());

    // Scrub and play  the transition
    Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
    Play(LeftPixel(VideoTransition(0,5)) - 5, 2200);

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
    ASSERT(VideoTransition(0,2)->getRight());
    ASSERT(!VideoTransition(0,2)->getLeft());

    // Move clip related to transition: the transition must be moved also
    DeselectAllClips();
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,4)->isA<model::Transition>());
    ASSERT(VideoTransition(0,4)->getRight());
    ASSERT(!VideoTransition(0,4)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());

    // Scrub and play the transition
    Scrub(LeftPixel(VideoTransition(0,4)) - 5, RightPixel(VideoTransition(0,4)) + 5);
    Play(LeftPixel(VideoTransition(0,4)) - 5, 2200);

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
    ASSERT(!VideoTransition(0,2)->getRight());
    ASSERT(VideoTransition(0,2)->getLeft());

    // Move clip related to transition: the transition must be moved also
    DeselectAllClips();
    Drag(Center(VideoClip(0,1)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT(!VideoTransition(0,5)->getRight());
    ASSERT(VideoTransition(0,5)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,8)->getRightPts(),AudioClip(0,7)->getRightPts());

    // Scrub and play the transition
    Scrub(LeftPixel(VideoTransition(0,5)) - 5, RightPixel(VideoTransition(0,5)) + 5);
    Play(LeftPixel(VideoTransition(0,5)) - 5, 2200);

    // Undo until the two trimmed clips are present
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::DeleteSelectedClips>();
    Undo();
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::Trim>();
}

} // namespace