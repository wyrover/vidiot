#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
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

    // This test ensures that when moving two clips around without selecting
    // the transition between them, that the transition is also dragged.

    // Make transition after clip 2
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

    // Move clip 2: the transition must be removed
    DeselectAllClips();
    Click(Center(VideoClip(0,1)));
    ASSERT_EQUALS(LogicalPosition(Center(VideoClip(0,3))), gui::timeline::ClipInterior);
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT(VideoTransition(0,5)->getRight());
    ASSERT(VideoTransition(0,5)->getLeft());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    DumpSequence();
    ASSERT_EQUALS(VideoClip(0,9)->getRightPts(),AudioClip(0,8)->getRightPts());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT(VideoTransition(0,5)->getRight());
    ASSERT(VideoTransition(0,5)->getLeft());

// todo playback moved transition (causes a bug since transition is not ended after last frame...)
// todo transition with only left and/or only right clip. and then scrubbing and/or playing and/or moving.
}

} // namespace