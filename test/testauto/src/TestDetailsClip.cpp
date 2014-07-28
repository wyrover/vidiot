// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "TestDetailsClip.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestDetailsClip::setUp()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; }
    ConfigOverruleLong overruleDefaultVideoWidth(Config::sPathDefaultVideoWidth, 720);
    ConfigOverruleLong overruleDefaultVideoHeight(Config::sPathDefaultVideoHeight, 576);
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

    auto ASSERT_ORIGINAL_CLIPPROPERTIES = []
    {
        ASSERT_CLIPPROPERTIES(
            VideoClip(0,3),
            model::VideoScalingFitToFill,
            boost::rational<int>(8000,model::Constants::sScalingPrecisionFactor),
            model::VideoAlignmentCenter,
            wxPoint(-152,0),
            0); };

    StartTest("If one clip is selected the details view changes accordingly.");
    TimelineLeftClick(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
    ASSERT_ORIGINAL_CLIPPROPERTIES();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    ASSERT_SELECTION_SIZE(1); // Clip and link selected
    pts originalLength = VideoClip(0,3)->getLength();

    auto getLength = [](wxToggleButton* button) -> pts { return model::Convert::timeToPts(button->GetId()); };
    auto pressLengthButton = [this,getLength] (wxToggleButton* button, bool enlarge, bool begin) -> int
    {
        pts oldLength = VideoClip(0,3)->getLength();
        std::ostringstream o;
        o << "LengthButton: " << (enlarge?"Enlarge":"Reduce") << " clip length (on " << (begin?"left":"right") << " side) to " << getLength(button);
        StartTest(o.str().c_str());
        waitForIdle();
        bool enabled = button->IsEnabled();
        RunInMainAndWait(boost::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed,DetailsClipView(),button));
        ASSERT_SELECTION_SIZE(1); // Clip and link selected
        if (enabled)
        {
            ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
            ASSERT_IMPLIES( enlarge, VideoClip(0,3)->getLength() >= oldLength)(VideoClip(0,3)->getLength())(oldLength);
            ASSERT_IMPLIES(!enlarge, VideoClip(0,3)->getLength() <= oldLength)(VideoClip(0,3)->getLength())(oldLength);
            ASSERT(VideoClip(0,3)->getSelected());
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), getLength(button));
            ASSERT_EQUALS(AudioClip(0,3)->getLength(), getLength(button));
            ASSERT_IMPLIES( begin, VideoClip(0,3)->getMaxAdjustEnd() == 0); // Check that the clip was trimmed at the begin
            ASSERT_IMPLIES( begin, AudioClip(0,3)->getMaxAdjustEnd() == 0); // Check that the clip was trimmed at the begin
            ASSERT_IMPLIES(!begin, VideoClip(0,3)->getMinAdjustBegin() == 0); // Check that the clip was trimmed at the end
            ASSERT_IMPLIES(!begin, AudioClip(0,3)->getMinAdjustBegin() == 0); // Check that the clip was trimmed at the end
        }
        else
        {
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), oldLength);
        }
        return enabled ? 1 : 0;
    };
    {
        // Test reducing the length on the right side (the default side)
        std::list<wxToggleButton*> buttons = DetailsClipView()->getLengthButtons(); // Can't use reverse on temporary inside for loop
        int nChanges = 0;
        for ( wxToggleButton* button : boost::adaptors::reverse(buttons) )
        {
            nChanges += pressLengthButton(button,false,false);
        }
        for ( wxToggleButton* button : buttons )
        {
            nChanges += pressLengthButton(button,true,false);
        }
        Undo(nChanges);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), originalLength);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), originalLength);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }

    {
        // Test reducing the length on the left side (can be triggered by overlapping the right side with a clip in another track)
        TriggerMenu(ID_ADDAUDIOTRACK);
        TriggerMenu(ID_ADDVIDEOTRACK);
        DragToTrack(1,VideoClip(0,5),AudioClip(0,5));

        for ( wxToggleButton* button : DetailsClipView()->getLengthButtons() )
        {
            Drag(From(Center(VideoClip(1,1))).AlignLeft(RightPixel(VideoClip(0,3)) - getTimeline().getZoom().ptsToPixels(getLength(button) -1)));
            TimelineLeftClick(Center(VideoClip(0,3)));
            for ( wxToggleButton* otherButton : DetailsClipView()->getLengthButtons() )
            {
                ASSERT_IMPLIES(getLength(otherButton) >= getLength(button), otherButton->IsEnabled())(getLength(otherButton))(getLength(button));
                ASSERT_IMPLIES(getLength(otherButton) <  getLength(button),!otherButton->IsEnabled())(getLength(otherButton))(getLength(button));
            }
             pressLengthButton(button,false,true);
            Undo(2); // Undo: adjust length, dragndrop
        }
        TrimLeft(VideoClip(0,3),getTimeline().getZoom().ptsToPixels(VideoClip(0,3)->getLength() - 2)); // Smaller length than the 'smallest' button
        TimelineLeftClick(Center(VideoClip(0,3))); // Exclusively select clip 4, since the shift trim above selects multiple clips
        for ( wxToggleButton* button : DetailsClipView()->getLengthButtons() )
        {
            ASSERT(button->IsEnabled());
            pressLengthButton(button,true,true);
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
        Undo(4); // Undo TrimLeft, ExecuteDrop, Add video track, Add audio track
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), originalLength);
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), originalLength);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
}

void TestDetailsClip::testChangeLengthOfTransition()
{
    StartTestSuite();
    Zoom level(6);
    auto getLength = [](wxToggleButton* button) -> pts { return model::Convert::timeToPts(button->GetId()); };
    auto pressLengthButtons = [this,getLength] ()
    {
        for ( wxToggleButton* button : DetailsClipView()->getLengthButtons() )
        {
            pts oldLength = VideoClip(0,2)->getLength();
            std::ostringstream o;
            o << "LengthButton: Change transition length to " << getLength(button);
            StartTest(o.str().c_str());
            RunInMainAndWait(boost::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed,DetailsClipView(),button));
            ASSERT_EQUALS(getSelectedClipsCount(),1); // Transition
            ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
            ASSERT_EQUALS(VideoClip(0,2)->getLength(),getLength(button));
            ASSERT(VideoClip(0,2)->getSelected());
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
    };

    {
        StartTest("InOutTransition: Change length via details view.");
        TrimRight(VideoClip(0,1),-100); // Make all lengths available
        TrimLeft(VideoClip(0,2),100); // Make all lengths available
        {
            MakeInOutTransitionAfterClip preparation(1);
            TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2))); // Select transition
            ASSERT_DETAILSCLIP(VideoClip(0,2));
            ASSERT(VideoClip(0,2)->isA<model::Transition>());
            ASSERT_EQUALS(getSelectedClipsCount(),1); // Transition
            pressLengthButtons();
        }
        Undo(2);
    }
    {
        StartTest("InTransition: Change length via details view.");
        MakeInTransitionAfterClip preparation(1);
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2))); // Select transition
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(getSelectedClipsCount(),1); // Transition
        pressLengthButtons();
    }
    {
        StartTest("OutTransition: Change length via details view.");
        MakeOutTransitionAfterClip preparation(1);
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2))); // Select transition
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(getSelectedClipsCount(),1); // Transition
        pressLengthButtons();
    }
}

void TestDetailsClip::testChangeLengthAfterCreatingTransition()
{
    StartTestSuite();
    Zoom level(2);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    auto getLength = [](wxToggleButton* button) -> pts { return model::Convert::timeToPts(button->GetId()); };
    auto pressLengthButtons = [this,getLength] (std::string name, pts minimumsize)
    {
        for ( wxToggleButton* button : DetailsClipView()->getLengthButtons() )
        {
            pts oldLength = VideoClip(0,2)->getPerceivedLength();
            pts buttonLength = getLength(button);
            bool buttonEnabled = button->IsEnabled();
            std::ostringstream o;
            o << name << ": length = " << buttonLength;
            StartTest(o.str().c_str());
            ASSERT_EQUALS(buttonEnabled, buttonLength >= minimumsize);
            RunInMainAndWait(boost::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed,DetailsClipView(),button));
            ASSERT_EQUALS(getSelectedClipsCount(),2);
            ASSERT(VideoClip(0,2)->getSelected());
            if (buttonEnabled)
            {
                ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
                ASSERT_EQUALS(VideoClip(0,2)->getPerceivedLength(), getLength(button));
                Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
            }
            else
            {
                ASSERT_EQUALS(VideoClip(0,2)->getPerceivedLength(), oldLength);
            }
        }
    };
    {
        StartTest("InTransition");
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('i');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("InTransition", defaultTransitionLength / 2);
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("OutTransition");
        OpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('o');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("OutTransition", defaultTransitionLength / 2);
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("InOutTransition");
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("InOutTransition", defaultTransitionLength / 2);
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("OutInTransition");
        OpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('n');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("OutInTransition", defaultTransitionLength / 2);
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("In+Out Transitions");
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o');
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('i');
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("In+Out Transitions", defaultTransitionLength);
        Undo(2); // Remove transitions
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("InOut+OutIn Transitions");
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("InOut+OutIn Transitions", defaultTransitionLength);
        Undo(2); // Remove transitions
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
}

void TestDetailsClip::testTransform()
{
    StartTestSuite();

    gui::Window::get().getUiManager().GetPane("Project").Hide();
    gui::Window::get().getUiManager().GetPane("Details").MinSize(wxSize(600,-1));

    model::VideoClipPtr videoclip = getVideoClip(VideoClip(0,3));
    model::VideoScaling oldScaling = videoclip->getScaling();
    boost::rational<int> oldScalingFactor = videoclip->getScalingFactor();
    wxPoint oldPosition = videoclip->getPosition();
    model::VideoAlignment oldAlignment = videoclip->getAlignment();
    boost::rational<int> oldRotation = videoclip->getRotation();
    ASSERT_EQUALS(videoclip->getInputSize(), wxSize(1280,720)); //Ensure that all checks are based on the right dimensions
    auto ASSERT_ORIGINAL_CLIPPROPERTIES = []
    {
        ASSERT_CLIPPROPERTIES(
            VideoClip(0,3),
            model::VideoScalingFitToFill,
            boost::rational<int>(8000,model::Constants::sScalingPrecisionFactor),
            model::VideoAlignmentCenter,
            wxPoint(-152,0),
            boost::rational<int>(0)); 
        // verify only one command is added to the history when doing multiple edits.
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>(); 
    };

    StartTest("If one clip is selected the details view changes accordingly.");
    TimelineLeftClick(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
    ASSERT_ORIGINAL_CLIPPROPERTIES();
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    {
        StartTest("Scaling: Slider: If moved to the right, the scaling is increased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSlider(), 7000); // Same as presing WXK_PAGEUP
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,boost::rational<int>(7000,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-88,36),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Slider: If moved to the left, the scaling is decreased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSlider(), 9000); // Same as presing WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,boost::rational<int>(9000,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-216,-36),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
//#ifdef __GNUC__
//    {
//        // Without this workaround, the first 'up' after giving the control focus increments with the wrong amount.
//        // Instead of incrementing the scaling with 0.0100, the scaling was increased with 1.0000.
//        StartTest("Workaround for GTK issue: First change of the scaling spin via the kbd changes with 1.0 i.s.o. 0.1");
//        GiveKeyboardFocus(DetailsClipView()->getScalingSpin());
//        Type(WXK_UP);
//        Undo();
//        ASSERT_ORIGINAL_CLIPPROPERTIES();
//    }
//#endif
    {
        StartTest("Scaling: Spin: If moved up, the scaling is increased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSpin(), DetailsClipView()->getScalingSpin()->GetValue() + 0.01); // Same as pressing WXK_UP
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor + boost::rational<int>(100,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-158,-3),oldRotation); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved down, the scaling is decreased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSpin(), DetailsClipView()->getScalingSpin()->GetValue() - 0.01); // Same as pressing WXK_DOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor - boost::rational<int>(100,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-145,4),oldRotation);// The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit all'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingFitAll);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitAll,boost::rational<int>(5625,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(0,85),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit to fill'");
        SetValue(DetailsClipView()->getScalingSlider(), 9000); // Same as presing WXK_PAGEDOWN. First change the position a bit.
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingFitToFill);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,boost::rational<int>(8000,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-152,0),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Original size'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingNone);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingNone,boost::rational<int>(10000,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-280,-72),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Custom'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingCustom);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor,oldAlignment,oldPosition,oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10),oldRotation);
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenter);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenter,wxPoint(-152,0),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center Horizontal'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10),oldRotation);
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterHorizontal);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center Vertical'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10),oldRotation);
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterVertical);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterVertical,wxPoint(-142,0),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Slider: moved to the right.");
        SetValue(DetailsClipView()->getRotationSlider(), -10); // Same as WXK_PAGEUP
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(-1,10));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Slider: moved to the left.");
        SetValue(DetailsClipView()->getRotationSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(1,10));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Spin: moved up.");
        SetValue(DetailsClipView()->getRotationSlider(), 1); // Same as WXK_UP
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(1,100));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: moved down.");
        SetValue(DetailsClipView()->getRotationSlider(), -1); // Same as WXK_DOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(-1,100));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Position cursor on center of clip, if the cursor was outside the clip's timeline region");
        PositionCursor(HCenter(VideoClip(0,1)));
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT_DETAILSCLIP(VideoClip(0,1));
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_DETAILSCLIP(VideoClip(0,3));
        SetValue(DetailsClipView()->getPositionYSlider(),10); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(CursorPosition(),HCenter(VideoClip(0,3))); // Now the cursor is moved to the center of the adjusted clip (for the preview)
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Keep cursor position, if the cursor was inside the clip's timeline region");
        pixel pos = HCenter(VideoClip(0,3)) - 20;
        PositionCursor(pos);
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT_DETAILSCLIP(VideoClip(0,1));
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_DETAILSCLIP(VideoClip(0,3));
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(CursorPosition(),pos); // Now the cursor is not moved: same frame is previewed
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
}

void TestDetailsClip::testTransform_Boundaries() // todo make the transform tests also work without physical gui events
{
    StartTestSuite();

    {
        StartTest("Scaling: Minimum scaling factor.");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT_DETAILSCLIP(VideoClip(0,5));
        GiveKeyboardFocus(DetailsClipView()->getScalingSpin());
        KeyboardKeyPressN(7,WXK_DELETE); // Remove all characters
        KeyboardKeyPress('0'); // 0 will be replaced with 'min' value
        KeyboardKeyPress(WXK_TAB);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,5),model::VideoScalingCustom,boost::rational<int>(model::Constants::sScalingMin,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(360,288),0); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
    }
    {
        StartTest("Scaling: Maximum scaling factor.");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT_DETAILSCLIP(VideoClip(0,5));
        GiveKeyboardFocus(DetailsClipView()->getScalingSpin());
        KeyboardKeyPressN(7,WXK_DELETE); // Remove all characters
        KeyboardKeyPressN(10,'9'); // 999999999 will be replaced with 'max' value
        KeyboardKeyPress(WXK_TAB);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,5),model::VideoScalingCustom,boost::rational<int>(model::Constants::sScalingMax,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-6040,-3312),0); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
    }
}

void TestDetailsClip::testChangeVolume()
{
    StartTestSuite();

    model::AudioCompositionParameters p = model::AudioCompositionParameters().setNrChannels(2).setSampleRate(44100);

    Unlink(AudioClip(0,3)); // Ensure that the audio controls are visible
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::UnlinkClips>();

    model::AudioClipPtr audioclip = getAudioClip(AudioClip(0,3));
    ASSERT_EQUALS(audioclip->getVolume(),100);
    audioclip->moveTo(10);
    model::AudioChunkPtr referenceChunk = audioclip->getNextAudio(p);
    ASSERT_EQUALS(audioclip->getVolume(),100);

    auto ASSERT_VOLUME = [referenceChunk,p,audioclip](int volume)
    {
        ASSERT_EQUALS(audioclip->getVolume(),volume);
        audioclip->moveTo(10);
        model::AudioChunkPtr audiochunk = audioclip->getNextAudio(p);

        sample* cur = audiochunk->getUnreadSamples();
        sample* ref = referenceChunk->getUnreadSamples();
        sample* last = cur + 100;
        while (cur < last)
        {
            int32_t c = *cur;
            int32_t r = *ref * volume / 100;
            ASSERT_EQUALS(c,r);
            cur++;
            ref++;
        }
    };

    TimelineLeftClick(Center(AudioClip(0,3)));

    {
        StartTest("Volume: Down via slider");
        GiveKeyboardFocus(DetailsClipView()->getVolumeSlider());
        KeyboardKeyPress(WXK_PAGEUP);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeAudioClipVolume>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(90);
        Undo();
        ASSERT_EQUALS(audioclip->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::UnlinkClips>();
    }
    {
        StartTest("Volume: Down via spin");
        GiveKeyboardFocus(DetailsClipView()->getVolumeSpin());
        KeyboardKeyPressN(4,WXK_DOWN); // Note: the click above is on the up arrow already triggering an increment of 1
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeAudioClipVolume>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(97);
        Undo();
        ASSERT_EQUALS(audioclip->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::UnlinkClips>();
    }
    {
        StartTest("Volume: Up via slider");
        GiveKeyboardFocus(DetailsClipView()->getVolumeSlider());
        KeyboardKeyPress(WXK_PAGEDOWN);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeAudioClipVolume>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(110);
        Undo();
        ASSERT_EQUALS(audioclip->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::UnlinkClips>();
    }
    {
        StartTest("Volume: Up via spin");
        GiveKeyboardFocus(DetailsClipView()->getVolumeSpin());
        KeyboardKeyPressN(4,WXK_UP); // Note: the click above is on the up arrow already triggering an increment of 1
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeAudioClipVolume>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(105);
        Undo();
        ASSERT_EQUALS(audioclip->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::UnlinkClips>();
    }
}

} // namespace