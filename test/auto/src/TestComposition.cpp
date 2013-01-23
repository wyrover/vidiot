#include "TestComposition.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "Convert.h"
#include "Details.h"
#include "DetailsClip.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperTimelineDrag.h"
#include "HelperModel.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "HelperWindow.h"
#include "ids.h"
#include "Layout.h"
#include "ProjectViewCreateSequence.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"

namespace test {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestComposition::setUp()
{
    mProjectFixture.init();
}

void TestComposition::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY(testBlend);
void TestComposition::testBlend()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);
    DragToTrack(1,VideoClip(0,3),model::IClipPtr());
    Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(3,WXK_PAGEUP);
    waitForIdle();
    ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    TypeN(4,WXK_PAGEDOWN);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(15,WXK_PAGEDOWN);
    ClickTopLeft(DetailsClipView()->getOpacitySlider()); // Give focus
    TypeN(6,WXK_PAGEUP);
    Click(Center(VideoClip(0,4)));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(5,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(8,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getOpacitySlider()); // Give focus
    TypeN(3,WXK_PAGEUP);
    Scrub(HCenter(VideoClip(0,4))-5,HCenter(VideoClip(0,4))+5);
    Play(HCenter(VideoClip(0,4)),100);
    Undo(6);
}

} // namespace