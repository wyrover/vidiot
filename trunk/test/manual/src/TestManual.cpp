#include "TestManual.h"

#include "ClipView.h"
#include "EmptyClip.h"
#include "FixtureGui.h"
#include "IClip.h"
#include "Selection.h"
#include "Sequence.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewCreateAutoFolder.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "HelperTimeline.h"
#include "HelperWindow.h"
#include "HelperProjectView.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "CreateTransition.h"
#include "UtilLog.h"
#include "ExecuteDrop.h"
#include "ViewMap.h"
#include "AutoFolder.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestManual::setUp()
{
    // Todo make defaultprojectfixture and remove helperprojectview
    TestFilesPath = wxFileName("D:\\Vidiot\\test", "");
    InputFiles = model::AutoFolder::getSupportedFiles(TestFilesPath);
    model::FolderPtr root = createProject();
    model::FolderPtr autofolder1 = addAutoFolder( TestFilesPath );
    model::SequencePtr sequence1 = createSequence( autofolder1 );
}

void TestManual::tearDown()
{
    // Must be done here, since the deletion of files causes logging. 
    // Logging is stopped (unavailable) after tearDown since application window is closed.
    InputFiles.clear(); 
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