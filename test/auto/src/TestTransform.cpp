#include "TestTransform.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "Constants.h"
#include "HelperWindow.h"
#include "Convert.h"
#include "Layout.h"
#include "HelperModel.h"
#include "Details.h"
#include "DetailsClip.h"
#include "VideoClip.h"
#include "ProjectViewCreateSequence.h"
#include "HelperWindow.h"
#include "UtilLogWxwidgets.h"

namespace test {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTransform::setUp()
{
    mProjectFixture.init();
}

void TestTransform::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

template <class ITEMTYPE>
void ClickOnEnumSelector(EnumSelector<ITEMTYPE>* widget, ITEMTYPE value)
{
    ClickTopLeft(widget);
    Type(WXK_HOME);
    for (int step = 0; step < widget->getIndex(value); ++step)
    {
        Type(WXK_DOWN);
    }
    Type(WXK_RETURN);
    ASSERT_EQUALS(widget->getValue(),value);
    waitForIdle();
}

RUNONLY("testTransform");
void TestTransform::testTransform()
{
    StartTestSuite();
    StartTest("If one clip is selected the details view changes accordingly.");
    Click(Center(VideoClip(0,3)));
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);

    auto ASSERT_CLIPPROPERTIES = [detailsclip] (model::VideoScaling scaling, int scalingdigits, model::VideoAlignment alignment, wxPoint position)
    {
        model::VideoClipPtr videoclip = getVideoClip(VideoClip(0,3));
        VAR_DEBUG(*videoclip);
        ASSERT_EQUALS(detailsclip->getScalingSlider()->GetValue(),scalingdigits);
        ASSERT_EQUALS(model::Convert::factorToDigits(detailsclip->getScalingSpin()->GetValue(),model::Constants::scalingPrecision),scalingdigits);
        ASSERT_EQUALS(detailsclip->getPositionXSlider()->GetValue(),position.x);
        ASSERT_EQUALS(detailsclip->getPositionXSpin()->GetValue(),position.x);
        ASSERT_EQUALS(detailsclip->getPositionYSlider()->GetValue(),position.y);
        ASSERT_EQUALS(detailsclip->getPositionYSpin()->GetValue(),position.y);
        ASSERT_EQUALS(videoclip->getScaling(),scaling);
        ASSERT_EQUALS(videoclip->getScalingDigits(),scalingdigits);
        ASSERT_EQUALS(videoclip->getAlignment(),alignment);
        ASSERT_EQUALS(videoclip->getPosition(),position);
    };
    model::VideoClipPtr videoclip = getVideoClip(VideoClip(0,3));
    model::VideoScaling oldScaling = videoclip->getScaling();
    int oldScalingDigits = videoclip->getScalingDigits();
    wxPoint oldPosition = videoclip->getPosition();
    model::VideoAlignment oldAlignment = videoclip->getAlignment();
    ASSERT_EQUALS(videoclip->getInputSize(), wxSize(1280,720)); //Ensure that all checks are based on the right dimensions
    auto ASSERT_ORIGINAL_CLIPPROPERTIES = [ASSERT_CLIPPROPERTIES] { ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,8000,model::VideoAlignmentCenter,wxPoint(-152,0)); };
    ASSERT_ORIGINAL_CLIPPROPERTIES();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    {
        StartTest("Scaling: Slider: If moved to the right, the scaling is increased. Scaling enum is changed to custom.");
        ClickTopLeft(detailsclip->getScalingSlider()); // Give focus
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
        Type(WXK_PAGEUP);
        ASSERT_CLIPPROPERTIES(model::VideoScalingCustom,7000,model::VideoAlignmentCenter,wxPoint(-88,36));
        Undo();
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>(); // Verify that only one command object was added to the undo history
        ASSERT_ORIGINAL_CLIPPROPERTIES();
        StartTest("Scaling: Slider: If moved to the left, the scaling is decreased. Scaling enum is changed to custom.");
        ClickTopLeft(detailsclip->getScalingSlider()); // Give focus
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingCustom,9000,model::VideoAlignmentCenter,wxPoint(-216,-36));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved up, the scaling is increased. Scaling enum is changed to custom.");
        ClickTopLeft(detailsclip->getScalingSpin(),wxPoint(2,2)); // Give focus
        Type(WXK_UP);
        ASSERT_CLIPPROPERTIES(model::VideoScalingCustom,oldScalingDigits + 100,model::VideoAlignmentCenter,wxPoint(-158,-3)); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
        StartTest("Scaling: Spin: If moved down, the scaling is decreased. Scaling enum is changed to custom.");
        Type(WXK_DOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingCustom,oldScalingDigits - 100,model::VideoAlignmentCenter,wxPoint(-145,4));// The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit all'");
        ClickOnEnumSelector(detailsclip->getScalingSelector(),model::VideoScalingFitAll);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitAll,5625,model::VideoAlignmentCenter,wxPoint(0,85));
        Undo();
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Fit to fill'");
        ClickTopLeft(detailsclip->getScalingSlider()); // First, set a different value than the defaults (which are already fit to fill)
        Type(WXK_PAGEDOWN);
        ClickOnEnumSelector(detailsclip->getScalingSelector(),model::VideoScalingFitToFill);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,8000,model::VideoAlignmentCenter,wxPoint(-152,0));
        Undo();
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Original size'");
        ClickOnEnumSelector(detailsclip->getScalingSelector(),model::VideoScalingNone);
        ASSERT_CLIPPROPERTIES(model::VideoScalingNone,10000,model::VideoAlignmentCenter,wxPoint(-280,-72));
        Undo();
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Custom'");
        ClickOnEnumSelector(detailsclip->getScalingSelector(),model::VideoScalingCustom);
        ASSERT_CLIPPROPERTIES(model::VideoScalingCustom,oldScalingDigits,oldAlignment,oldPosition);
        Undo();
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center'");
        ClickTopLeft(detailsclip->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ClickTopLeft(detailsclip->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(detailsclip->getAlignmentSelector(),model::VideoAlignmentCenter);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenter,wxPoint(-152,0));
        Undo();
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center Horizontal'");
        ClickTopLeft(detailsclip->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10)); // Test that the alignment is changed from center to centerhorizontal automatically
        ClickTopLeft(detailsclip->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(detailsclip->getAlignmentSelector(),model::VideoAlignmentCenterHorizontal);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10));
        Undo();
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center Vertical'");
        ClickTopLeft(detailsclip->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterVertical,wxPoint(-142,0)); // Test that the alignment is changed from center to centervertical automatically
        ClickTopLeft(detailsclip->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(detailsclip->getAlignmentSelector(),model::VideoAlignmentCenterVertical);
        ASSERT_CLIPPROPERTIES(model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterVertical,wxPoint(-142,0));
        wxCommand* command = getCurrentCommand();
        Undo();
        ASSERT_DIFFERS(command,getCurrentCommand());
        ASSERT_CLIPPROPERTIES(oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
}

} // namespace