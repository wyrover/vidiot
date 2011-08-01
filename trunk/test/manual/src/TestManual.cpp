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
#include "VideoTrack.h"
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

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Type('c');
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 2: the transition must be removed
    DeselectAllClips();
    Click(VideoClip(0,1));
    Drag(Center(VideoClip(0,3)), Center(VideoClip(0,5)), true);
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,5)->isA<model::Transition>());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    DumpSequence();
    ASSERT_EQUALS(VideoClip(0,9)->getRightPts(),AudioClip(0,8)->getRightPts());
    pause();
    BOOST_FOREACH( model::IClipPtr clip, VideoTrack(0)->getClips() )
    {
        ASSERT(!clip->isA<model::Transition>());
    }

pause();

// todo playback moved transition (causes a bug since transition is not ended after last frame...)
// todo during drag the transition is shown but too far to the right...
}

} // namespace