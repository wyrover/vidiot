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

    // This test ensures that when moving two clips around without selecting
    // the transition between them, that the transition is also dragged.

    // Zoom in
//    wxUIActionSimulator().Char('=');
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

}

} // namespace