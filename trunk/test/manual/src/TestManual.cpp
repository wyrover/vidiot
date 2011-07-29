#include "TestManual.h"

#include <wx/uiaction.h>
#include "AutoFolder.h"
#include "ClipView.h"
#include "CreateTransition.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "IClip.h"
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
    getTimeline().getSelection().unselectAll();
    Drag(Center(VideoClip(0,1)), Center(VideoClip(0,4)));
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());

    triggerUndo();
    ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 3: the transition must be removed and the fourth clip becomes the third one (clip+transition removed)
    model::IClipPtr afterclip = VideoClip(0,4);
    getTimeline().getSelection().unselectAll();
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