#include "TestManual.h"

#include "Dialog.h"
#include "DetailsClip.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperModel.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ProjectView.h"
#include "ids.h"
#include "Layout.h"
#include "File.h"
#include "Logging.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "UtilLogWxwidgets.h"
#include <boost/foreach.hpp>
#include <wx/uiaction.h>

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TestManual::TestManual()
    :   mProjectFixture(false) // Set to true if the timeline must be focused initially
{
}

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
    StartTestSuite();
    StartTest("Manual...");
    LOG_ERROR << dump(getSequence());

    //triggerMenu(ID_ADDVIDEOTRACK);
    //triggerMenu(ID_ADDVIDEOTRACK);

    //DragToTrack(1,VideoClip(0,5),model::IClipPtr());
    //Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));

    //DragToTrack(2,VideoClip(0,6),model::IClipPtr());
    //Drag(From(Center(VideoClip(2,1))).AlignLeft(LeftPixel(VideoClip(1,1))));
    //    //.To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(2,1)))));

    //Click(Center(VideoClip(1,1)));
    //ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    //TypeN(6,WXK_PAGEUP);
    //waitForIdle();
    //ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    //TypeN(4,WXK_PAGEDOWN);
    //waitForIdle();
    //ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    //TypeN(4,WXK_PAGEDOWN);

    //Click(Center(VideoClip(2,1)));
    //ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    //TypeN(6,WXK_PAGEUP);
    //waitForIdle();
    //ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    //TypeN(4,WXK_PAGEUP);
    //waitForIdle();
    //ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    //TypeN(4,WXK_PAGEUP);

    //PositionCursor(LeftPixel(VideoClip(2,1)) - 5);

    //Play(LeftPixel(VideoClip(2,1)) - 3,2500); // Start before the clip, in the empty area. Due to a bug in 'getNext' handling for empty clips the clips after the empty area were not shown, or too late.
    //Undo(9);

    // todo verdeling bij audio clip details: grow sliders much more
    // todo test on machine without msvc, since two ms dlls are installed in bin/

    // todo no audio in rendered file...

    // todo make details drag to show hints during dragging (see trim...)

    // todo at end of clip edit, remove any empty clips which are the last clips of the track
    pause(60000 * 60); // One hour should be enough...
}

} // namespace