#include "TestTransform.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "Convert.h"
#include "Details.h"
#include "DetailsClip.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperModel.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperTransform.h"
#include "HelperWindow.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "ProjectViewCreateSequence.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"

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

void TestTransform::testTransformViaDetailsView()
{
    StartTestSuite();

    model::VideoClipPtr videoclip = getVideoClip(VideoClip(0,3));
    model::VideoScaling oldScaling = videoclip->getScaling();
    int oldScalingDigits = videoclip->getScalingDigits();
    wxPoint oldPosition = videoclip->getPosition();
    model::VideoAlignment oldAlignment = videoclip->getAlignment();
    ASSERT_EQUALS(videoclip->getInputSize(), wxSize(1280,720)); //Ensure that all checks are based on the right dimensions
    auto ASSERT_ORIGINAL_CLIPPROPERTIES = [] { ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,8000,model::VideoAlignmentCenter,wxPoint(-152,0)); };

    StartTest("If one clip is selected the details view changes accordingly.");
    Click(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
    ASSERT_ORIGINAL_CLIPPROPERTIES();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    {
        StartTest("Scaling: Slider: If moved to the right, the scaling is increased. Scaling enum is changed to custom.");
        ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
        Type(WXK_PAGEUP);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,7000,model::VideoAlignmentCenter,wxPoint(-88,36));
        Undo();
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>(); // Verify that only one command object was added to the undo history
        ASSERT_ORIGINAL_CLIPPROPERTIES();
        StartTest("Scaling: Slider: If moved to the left, the scaling is decreased. Scaling enum is changed to custom.");
        ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,9000,model::VideoAlignmentCenter,wxPoint(-216,-36));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved up, the scaling is increased. Scaling enum is changed to custom.");
        ClickTopLeft(DetailsClipView()->getScalingSpin(),wxPoint(2,2)); // Give focus
        Type(WXK_UP);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingDigits + 100,model::VideoAlignmentCenter,wxPoint(-158,-3)); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
        StartTest("Scaling: Spin: If moved down, the scaling is decreased. Scaling enum is changed to custom.");
        Type(WXK_DOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingDigits - 100,model::VideoAlignmentCenter,wxPoint(-145,4));// The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit all'");
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingFitAll);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitAll,5625,model::VideoAlignmentCenter,wxPoint(0,85));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Fit to fill'");
        ClickTopLeft(DetailsClipView()->getScalingSlider()); // First, set a different value than the defaults (which are already fit to fill)
        Type(WXK_PAGEDOWN);
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingFitToFill);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,8000,model::VideoAlignmentCenter,wxPoint(-152,0));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Original size'");
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingNone);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingNone,10000,model::VideoAlignmentCenter,wxPoint(-280,-72));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Custom'");
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingCustom);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingDigits,oldAlignment,oldPosition);
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center'");
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenter);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenter,wxPoint(-152,0));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center Horizontal'");
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10)); // Test that the alignment is changed from center to centerhorizontal automatically
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterHorizontal);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center Vertical'");
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterVertical,wxPoint(-142,0)); // Test that the alignment is changed from center to centervertical automatically
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterVertical);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingDigits,model::VideoAlignmentCenterVertical,wxPoint(-142,0));
        wxCommand* command = getCurrentCommand();
        Undo();
        ASSERT_DIFFERS(command,getCurrentCommand());
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingDigits,oldAlignment,oldPosition);
    }
    {
        StartTest("Position cursor on center of clip, if the cursor was outside the clip's timeline region");
        PositionCursor(HCenter(VideoClip(0,1)));
        Click(Center(VideoClip(0,1)));
        ASSERT_DETAILSCLIP(VideoClip(0,1));
        Click(Center(VideoClip(0,4)));
        ASSERT_DETAILSCLIP(VideoClip(0,4));
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Change the clip
        Type(WXK_PAGEDOWN);
        ASSERT_EQUALS(CursorPosition(),HCenter(VideoClip(0,4))); // Now the cursor is moved to the center of the adjusted clip (for the preview)
    }
    {
        StartTest("Keep cursor position, if the cursor was inside the clip's timeline region");
        pixel pos = HCenter(VideoClip(0,4)) - 40;
        PositionCursor(pos);
        Click(Center(VideoClip(0,1)));
        ASSERT_DETAILSCLIP(VideoClip(0,1));
        Click(Center(VideoClip(0,4)));
        ASSERT_DETAILSCLIP(VideoClip(0,4));
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Change the clip
        Type(WXK_PAGEDOWN);
        ASSERT_EQUALS(CursorPosition(),pos); // Now the cursor is not moved: same frame is previewed
    }
}

} // namespace