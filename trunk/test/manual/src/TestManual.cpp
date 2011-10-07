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
    // rename to testSplitting

    // Make transition before clip 3
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd())(VideoClip(0,1));
    ASSERT_LESS_THAN_ZERO(VideoClip(0,2)->getMinAdjustBegin())(VideoClip(0,2));
    PositionCursor(LeftPixel(VideoClip(0,2)));
    Move(LeftCenter(VideoClip(0,2)));
    //pause();
    Type('c');
    ASSERT(VideoClip(0,2)->isA<model::Transition>())(VideoClip(0,2));

    DeselectAllClips();
    Click(Center(VideoClip(0,1)));
    wxPoint from = LeftCenter(VideoClip(0,6));
    from.x += 10;
    wxPoint to = Center(VideoClip(0,2));
    Drag(from, to, false);

                //todo this now goes wrong because originallink has been removed completely (obscured with dropped clip)
                //make test case specific for this


    pause();


}

} // namespace