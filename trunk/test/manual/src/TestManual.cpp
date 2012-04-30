#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "HelperModel.h"
#include "Details.h"
#include "DetailsClip.h"
#include "VideoClip.h"

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
    StartTest("If one clip is selected the details view changes accordingly.");
    Click(Center(VideoClip(0,3)));
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);
    ASSERT_EQUALS(getScaling(VideoClip(0,3)),model::VideoScalingFitToFill);

    ClickTopLeft(detailsclip->getScalingSlider()); // Give focus
    double oldfactor = getScalingFactor(VideoClip(0,3));
    StartTest("If the scaling slider is moved to the right, the scaling is increased. Also test that the scaling is changed to custom.");
    Type(WXK_RIGHT);
    ASSERT_EQUALS(getScalingFactor(VideoClip(0,3)),oldfactor + gui::Layout::sScalingIncrement);
    ASSERT_EQUALS(getScaling(VideoClip(0,3)),model::VideoScalingCustom);
    StartTest("If the scaling slider is moved to the left, the scaling is decreased.");
    Type(WXK_LEFT);
    ASSERT_EQUALS(getScalingFactor(VideoClip(0,3)),oldfactor);
    ASSERT_EQUALS(getScaling(VideoClip(0,3)),model::VideoScalingCustom);

    ClickTopLeft(detailsclip->getScalingSpin(),wxPoint(2,2)); // Give focus
    StartTest("If the spin ctrl is moved up, the scaling is increased.");
    Type(WXK_UP);
    ASSERT_EQUALS(getScalingFactor(VideoClip(0,3)),oldfactor + gui::Layout::sScalingIncrement);
    ASSERT_EQUALS(getScaling(VideoClip(0,3)),model::VideoScalingCustom);
    StartTest("If the spin ctrl is moved down, the scaling is decreased.");
    Type(WXK_DOWN);
    ASSERT_EQUALS(getScalingFactor(VideoClip(0,3)),oldfactor);
    ASSERT_EQUALS(getScaling(VideoClip(0,3)),model::VideoScalingCustom);

    StartTest("Selecting a specific scaling type.");
    int steps = 0;
    BOOST_FOREACH( model::VideoScalingConverter::VideoScalingMap::value_type value, model::VideoScalingConverter::mapToHumanReadibleString)
    {
        ClickTopLeft(detailsclip->getScalingChoice());
        Type(WXK_HOME);
        for (int step = 0; step < steps; ++step)
        {
            Type(WXK_DOWN);
        }
        Type(WXK_RETURN);
        ASSERT_EQUALS(getScaling(VideoClip(0,3)), value.left);
        ++steps;
    }

    pause(60000 * 60); // One hour should be enough...
}
} // namespace