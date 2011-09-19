#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "Dialog.h"
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
#include "Window.h"
#include "ids.h"

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
    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Type('c');
    waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());


}

} // namespace