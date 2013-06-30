#include "TestDetailsClip.h"

#include "ChangeVideoClipTransform.h"
#include "Constants.h"
#include "Convert.h"
#include "DetailsClip.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperModel.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperTransform.h"
#include "HelperWindow.h"
#include "ids.h"
#include "ProjectViewCreateSequence.h"
#include "Timeline.h"
#include "TrimClip.h"
#include "UtilLogWxwidgets.h"
#include "UtilThread.h"
#include "VideoClip.h"
#include "Zoom.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestDetailsClip::setUp()
{
    mProjectFixture.init();
}

void TestDetailsClip::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestDetailsClip::testChangeLength()
{
    StartTestSuite();
    Zoom level(2);

    auto ASSERT_ORIGINAL_CLIPPROPERTIES = [] { ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,boost::rational<int>(8000,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-152,0)); };

    StartTest("If one clip is selected the details view changes accordingly.");
    Click(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
    ASSERT_ORIGINAL_CLIPPROPERTIES();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    ASSERT_SELECTION_SIZE(1); // Clip and link selected
    pts originalLength = VideoClip(0,3)->getLength();

    auto getLength = [](wxToggleButton* button) -> pts { return model::Convert::timeToPts(button->GetId()); };
    auto pressLengthButton = [this,getLength] (wxToggleButton* button, bool enlarge, bool begin)
    {
        pts oldLength = VideoClip(0,3)->getLength();
        std::ostringstream o;
        o << "LengthButton: " << (enlarge?"Enlarge":"Reduce") << " clip length (on " << (begin?"left":"right") << " side) to " << getLength(button);
        StartTest(o.str().c_str());
        waitForIdle();
        util::thread::RunInMainAndWait(boost::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed,DetailsClipView(),button));
        waitForIdle();
        ASSERT_SELECTION_SIZE(1); // Clip and link selected
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
        ASSERT_IMPLIES( enlarge, VideoClip(0,3)->getLength() >= oldLength)(VideoClip(0,3)->getLength())(oldLength);
        ASSERT_IMPLIES(!enlarge, VideoClip(0,3)->getLength() <= oldLength)(VideoClip(0,3)->getLength())(oldLength);
        ASSERT(VideoClip(0,3)->getSelected());
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), getLength(button));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), getLength(button));
        ASSERT_IMPLIES( begin, VideoClip(0,3)->getMaxAdjustEnd() == 0); // Check that the clip was trimmed at the begin
        ASSERT_IMPLIES( begin, AudioClip(0,3)->getMaxAdjustEnd() == 0); // Check that the clip was trimmed at the begin
        ASSERT_IMPLIES(!begin, VideoClip(0,3)->getMinAdjustBegin() == 0); // Check that the clip was trimmed at the end
        ASSERT_IMPLIES(!begin, AudioClip(0,3)->getMinAdjustBegin() == 0); // Check that the clip was trimmed at the end
    };

    {
        // Test reducing the length on the right side (the default side)
        BOOST_REVERSE_FOREACH( wxToggleButton* button,  DetailsClipView()->getLengthButtons() )
        {
            pressLengthButton(button,false,false);
        }
        BOOST_FOREACH( wxToggleButton* button,  DetailsClipView()->getLengthButtons() )
        {
            pressLengthButton(button,true,false);
        }
        Undo(DetailsClipView()->getLengthButtons().size() * 2);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), originalLength);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), originalLength);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }

    {
        // Test reducing the length on the left side (can be triggered by overlapping the right side with a clip in another track)
        triggerMenu(ID_ADDAUDIOTRACK);
        triggerMenu(ID_ADDVIDEOTRACK);
        DragToTrack(1,VideoClip(0,5),AudioClip(0,5));

        BOOST_FOREACH( wxToggleButton* button, DetailsClipView()->getLengthButtons() )
        {
            Drag(From(Center(VideoClip(1,1))).AlignLeft(RightPixel(VideoClip(0,3)) - getTimeline().getZoom().ptsToPixels(getLength(button) -1)));
            Click(Center(VideoClip(0,3)));
            BOOST_FOREACH( wxToggleButton* otherButton, DetailsClipView()->getLengthButtons() )
            {
                ASSERT_IMPLIES(getLength(otherButton) >= getLength(button), otherButton->IsEnabled())(getLength(otherButton))(getLength(button));
                ASSERT_IMPLIES(getLength(otherButton) <  getLength(button),!otherButton->IsEnabled())(getLength(otherButton))(getLength(button));
            }
             pressLengthButton(button,false,true);
            Undo(2); // Undo: adjust length, dragndrop
        }
        TrimLeft(VideoClip(0,3),getTimeline().getZoom().ptsToPixels(VideoClip(0,3)->getLength() - 5)); // Smaller length than the 'smallest' button
        Click(Center(VideoClip(0,3))); // Exclusively select clip 4, since the shift trim above selects multiple clips
        BOOST_FOREACH( wxToggleButton* button, DetailsClipView()->getLengthButtons() )
        {
            ASSERT(button->IsEnabled());
        }
        BOOST_FOREACH( wxToggleButton* button, DetailsClipView()->getLengthButtons() )
        {
            pressLengthButton(button,true,true);
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
        Undo(4); // Undo TrimLeft, ExecuteDrop, Add video track, Add audio track
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), originalLength);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), originalLength);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
}

void TestDetailsClip::testTransform()
{
    StartTestSuite();

    model::VideoClipPtr videoclip = getVideoClip(VideoClip(0,3));
    model::VideoScaling oldScaling = videoclip->getScaling();
    boost::rational<int> oldScalingFactor = videoclip->getScalingFactor();
    wxPoint oldPosition = videoclip->getPosition();
    model::VideoAlignment oldAlignment = videoclip->getAlignment();
    ASSERT_EQUALS(videoclip->getInputSize(), wxSize(1280,720)); //Ensure that all checks are based on the right dimensions
    auto ASSERT_ORIGINAL_CLIPPROPERTIES = [] { ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,boost::rational<int>(8000,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-152,0)); };

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
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,boost::rational<int>(7000,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-88,36));
        Undo();
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>(); // Verify that only one command object was added to the undo history
        ASSERT_ORIGINAL_CLIPPROPERTIES();
        StartTest("Scaling: Slider: If moved to the left, the scaling is decreased. Scaling enum is changed to custom.");
        ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,boost::rational<int>(9000,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-216,-36));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved up, the scaling is increased. Scaling enum is changed to custom.");
        ClickTopLeft(DetailsClipView()->getScalingSpin(),wxPoint(2,2)); // Give focus
        Type(WXK_UP);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor + boost::rational<int>(100,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-158,-3)); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
        StartTest("Scaling: Spin: If moved down, the scaling is decreased. Scaling enum is changed to custom.");
        Type(WXK_DOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor - boost::rational<int>(100,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-145,4));// The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit all'");
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingFitAll);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitAll,boost::rational<int>(5625,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(0,85));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Fit to fill'");
        ClickTopLeft(DetailsClipView()->getScalingSlider()); // First, set a different value than the defaults (which are already fit to fill)
        Type(WXK_PAGEDOWN);
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingFitToFill);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,boost::rational<int>(8000,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-152,0));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Original size'");
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingNone);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingNone,boost::rational<int>(10000,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-280,-72));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
    }
    {
        StartTest("Scaling: Choice: 'Custom'");
        ClickOnEnumSelector(DetailsClipView()->getScalingSelector(),model::VideoScalingCustom);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor,oldAlignment,oldPosition);
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center'");
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenter);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenter,wxPoint(-152,0));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center Horizontal'");
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10)); // Test that the alignment is changed from center to centerhorizontal automatically
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterHorizontal);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10));
        Undo();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
    }
    {
        StartTest("Alignment: Choice: 'Center Vertical'");
        ClickTopLeft(DetailsClipView()->getPositionXSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterVertical,wxPoint(-142,0)); // Test that the alignment is changed from center to centervertical automatically
        ClickTopLeft(DetailsClipView()->getPositionYSlider()); // First, set different values
        Type(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10));
        ClickOnEnumSelector(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterVertical);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterVertical,wxPoint(-142,0));
        wxCommand* command = getCurrentCommand();
        Undo();
        ASSERT_DIFFERS(command,getCurrentCommand());
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition);
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
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
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
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(CursorPosition(),pos); // Now the cursor is not moved: same frame is previewed
    }
}

void TestDetailsClip::testTransform_Boundaries()
{
    StartTestSuite();

    {
        StartTest("Scaling: Minimum scaling factor.");
        Click(Center(VideoClip(0,5)));
        ASSERT_DETAILSCLIP(VideoClip(0,5));
        ClickTopLeft(DetailsClipView()->getScalingSpin(),wxPoint(2,2)); // Give focus
        TypeN(7,WXK_DELETE); // Remove all characters
        Type('0'); // 0 will be replaced with 'min' value
        Type(WXK_TAB);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,5),model::VideoScalingCustom,boost::rational<int>(model::Constants::sMinScaling,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(360,288)); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
    }
    {
        StartTest("Scaling: Maximum scaling factor.");
        Click(Center(VideoClip(0,5)));
        ASSERT_DETAILSCLIP(VideoClip(0,5));
        ClickTopLeft(DetailsClipView()->getScalingSpin(),wxPoint(2,2)); // Give focus
        TypeN(7,WXK_DELETE); // Remove all characters
        TypeN(10,'9'); // 999999999 will be replaced with 'max' value
        Type(WXK_TAB);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,5),model::VideoScalingCustom,boost::rational<int>(model::Constants::sMaxScaling,model::Constants::scalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-6040,-3312)); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
    }
}

} // namespace