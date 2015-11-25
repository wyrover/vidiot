// Copyright 2013-2015 Eric Raijmakers.
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
    ConfigOverrule<long> overruleDefaultVideoWidth(Config::sPathVideoDefaultWidth, 720);
    ConfigOverrule<long> overruleDefaultVideoHeight(Config::sPathVideoDefaultHeight, 576);
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
    TimelineZoomIn(2);
    WindowTriggerMenu(ID_SHOW_PROJECT);

    StartTest("If one clip is selected the details view changes accordingly.");
    TimelineLeftClick(Center(VideoClip(0, 3)));
    ASSERT_DETAILSCLIP(VideoClip(0, 3));
        ASSERT_CLIPPROPERTIES(
            VideoClip(0, 3),
            model::VideoScalingFitToFill,
            boost::rational<int>(4, 5),
            model::VideoAlignmentCenter,
            wxPoint(-152, 0),
            0);
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    ASSERT_SELECTION_SIZE(1); // Clip and link selected
    pts originalLength = VideoClip(0, 3)->getLength();

    auto pressLengthButton = [this](wxToggleButton* button, bool enlarge, bool begin) -> int
    {
        pts oldLength = VideoClip(0, 3)->getLength();
        std::ostringstream o;
        o << "LengthButton: " << (enlarge ? "Enlarge" : "Reduce") << " clip length (on " << (begin ? "left" : "right") << " side) to " << DetailsClipView()->getLength(button);
        StartTest(o.str().c_str());
        WaitForIdle;
        bool enabled = button->IsEnabled();
        ASSERT_SELECTION_SIZE(1); // Clip and link selected
        if (enabled)
        {
            util::thread::RunInMainAndWait(std::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed, DetailsClipView(), button));
            ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
            ASSERT_IMPLIES(enlarge, VideoClip(0, 3)->getLength() >= oldLength)(VideoClip(0, 3)->getLength())(oldLength);
            ASSERT_IMPLIES(!enlarge, VideoClip(0, 3)->getLength() <= oldLength)(VideoClip(0, 3)->getLength())(oldLength);
            ASSERT(VideoClip(0, 3)->getSelected());
            ASSERT_EQUALS(VideoClip(0, 3)->getLength(), DetailsClipView()->getLength(button));
            ASSERT_EQUALS(AudioClip(0, 3)->getLength(), DetailsClipView()->getLength(button));
            ASSERT_IMPLIES(begin, VideoClip(0, 3)->getMaxAdjustEnd() == 0); // Check that the clip was trimmed at the begin
            ASSERT_IMPLIES(begin, AudioClip(0, 3)->getMaxAdjustEnd() == 0); // Check that the clip was trimmed at the begin
            ASSERT_IMPLIES(!begin, VideoClip(0, 3)->getMinAdjustBegin() == 0); // Check that the clip was trimmed at the end
            ASSERT_IMPLIES(!begin, AudioClip(0, 3)->getMinAdjustBegin() == 0); // Check that the clip was trimmed at the end
        }
        return enabled ? 1 : 0;
    };
    {
        // Test reducing the length on the right side (the default side)
        std::vector<wxToggleButton*> buttons = DetailsClipView()->getLengthButtons(); // Can't use reverse on temporary inside for loop
        int nChanges = 0;
        for (wxToggleButton* button : boost::adaptors::reverse(buttons))
        {
            nChanges += pressLengthButton(button, false, false);
        }
        for (wxToggleButton* button : buttons)
        {
            nChanges += pressLengthButton(button, true, false);
        }
        Undo(nChanges);
        ASSERT_EQUALS(VideoClip(0, 3)->getLength(), originalLength);
        ASSERT_EQUALS(AudioClip(0, 3)->getLength(), originalLength);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }

    {
        StartTest("Test reducing the length on the left side (triggered by overlapping right side with clip in another track.");
        WindowTriggerMenu(ID_ADDAUDIOTRACK);
        WindowTriggerMenu(ID_ADDVIDEOTRACK);
        TimelineDragToTrack(1, VideoClip(0, 5), AudioClip(0, 5));
        for (int index : { 0, 1, 2, 3, 4, 5, 6, 7 }) // Not all sizes possible (10s too large too 'fill up' on the right side and then trim on the left side - would need a 20s clip for that)
        {
            wxToggleButton* button = DetailsClipView()->getLengthButtons()[index];
            pts length{ DetailsClipView()->getLength(button) };
            TimelineDrag(From(Center(VideoClip(1, 1))).AlignLeft(RightPixel(VideoClip(0, 3)) - getTimeline().getZoom().ptsToPixels(length)));
            TimelineLeftClick(Center(VideoClip(0, 3)));
            ASSERT(button->IsEnabled());
            for (wxToggleButton* otherButton : DetailsClipView()->getLengthButtons())
            {
                pts otherLength{ DetailsClipView()->getLength(otherButton) };
                ASSERT_IMPLIES(otherLength < length, !otherButton->IsEnabled())(otherLength)(length);
            }
            pressLengthButton(button, false, true);
            Undo(2); // Undo: adjust length, dragndrop
        }
        TimelineTrimLeft(VideoClip(0, 3), getTimeline().getZoom().ptsToPixels(VideoClip(0, 3)->getLength() - 2)); // Smaller length than the 'smallest' button
        TimelineLeftClick(Center(VideoClip(0, 3))); // Exclusively select clip 4, since the shift trim above selects multiple clips
        for (wxToggleButton* button : DetailsClipView()->getLengthButtons())
        {
            ASSERT(button->IsEnabled());
            pressLengthButton(button, true, true);
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
        Undo(4); // Undo TimelineTrimLeft, ExecuteDrop, Add video track, Add audio track
        ASSERT_EQUALS(VideoClip(0, 3)->getLength(), originalLength);
        ASSERT_EQUALS(AudioClip(0, 3)->getLength(), originalLength);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
}

void TestDetailsClip::testChangeLengthTooMuch()
{
    StartTestSuite();
    TimelineZoomIn(5);
    WindowTriggerMenu(ID_SHOW_PROJECT);

    MakeInOutTransitionAfterClip outTransition(1,true);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(Transition)(AudioClip);
    MakeInOutTransitionAfterClip inTransition(0,true);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(Transition)(AudioClip)(Transition)(AudioClip);
    TimelineLeftClick(Center(AudioClip(0,2))); // Selects both the video and the audio clip

    model::IClipPtr clip = AudioClip(0,2);
    pts minLength = *(clip->getInTransition()->getRight()) + *(clip->getOutTransition()->getLeft());
    pts maxLength = clip->getLength() + clip->getMinAdjustBegin() + clip->getMaxAdjustEnd();

    for (int index : { 0, 1, 8 })
    {
        ASSERT(!DetailsClipView()->getLengthButtons()[index]->IsEnabled());
    }

    for (int index : {2, 3, 4, 5, 6, 7})
    {
        wxToggleButton* button = DetailsClipView()->getLengthButtons()[index];
        pts buttonlength = DetailsClipView()->getLength(button);
        ASSERT(button->IsEnabled());
        util::thread::RunInMainAndWait(std::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed, DetailsClipView(), button));
        ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
        ASSERT_EQUALS(VideoClip(0, 1)->getLength(), buttonlength); // Note: video clip has the same length, audio clip is smaller
        Undo();
    }
}

void TestDetailsClip::testChangeLengthOfTransition()
{
    StartTestSuite();
    TimelineZoomIn(6);
    WindowTriggerMenu(ID_SHOW_PROJECT);

    auto selectClipAndPressLengthButtons = [this] (std::vector<int> enabledButtons, std::vector<int> disabledButtons)
    {
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2))); // Select transition
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT_EQUALS(getSelectedClipsCount(),1); // Transition

        for (int index : disabledButtons)
        {
            ASSERT(!DetailsClipView()->getLengthButtons()[index]->IsEnabled());
        }

        for (int index : enabledButtons)
        {
            wxToggleButton* button = DetailsClipView()->getLengthButtons()[index];
            pts length = DetailsClipView()->getLength(button);
            ASSERT(button->IsEnabled());
            pts oldLength = VideoClip(0, 2)->getLength();
            std::ostringstream o;
            o << "LengthButton: Change transition length to " << length;
            StartTest(o.str().c_str());
            util::thread::RunInMainAndWait(std::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed, DetailsClipView(), button));
            ASSERT_EQUALS(getSelectedClipsCount(), 1); // Transition
            ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
            ASSERT_EQUALS(VideoClip(0, 2)->getLength(), length);
            ASSERT(VideoClip(0, 2)->getSelected());
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
    };

    {
        StartTest("InOutTransition: Change length via details view.");
        TimelineTrimRight(VideoClip(0,1),-100); // Make lengths available
        TimelineTrimLeft(VideoClip(0,2),100); // Make lengths available
        {
            MakeInOutTransitionAfterClip preparation(1);
            selectClipAndPressLengthButtons({ 0, 1, 2, 3, 4, 5, 6, 7 }, { 8 }); // 10s button is disabled
        }
        Undo(2);
        TimelineDeselectAllClips();
    }
    {
        StartTest("InTransition: Change length via details view.");
        MakeInTransitionAfterClip preparation(1);
        selectClipAndPressLengthButtons({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }, {});
    }
    {
        StartTest("OutTransition: Change length via details view.");
        MakeOutTransitionAfterClip preparation(1);
        selectClipAndPressLengthButtons({ 0, 1, 2, 3, 4, 5, 6, 7 }, { 8 }); // 10s button is disabled
    }
}

void TestDetailsClip::testChangeLengthAfterCreatingTransition()
{
    StartTestSuite();
    TimelineZoomIn(2);
    WindowTriggerMenu(ID_SHOW_PROJECT);

    auto pressLengthButtons = [this] (std::string name, std::vector<int> enabledButtons, std::vector<int> disabledButtons)
    {
        for (int index : disabledButtons)
        {
            ASSERT(!DetailsClipView()->getLengthButtons()[index]->IsEnabled());
        }
        for (int index : enabledButtons)
        {
            wxToggleButton* button = DetailsClipView()->getLengthButtons()[index];
            pts oldLength = VideoClip(0,2)->getPerceivedLength();
            pts buttonLength = DetailsClipView()->getLength(button);
            ASSERT(button->IsEnabled());
            std::ostringstream o;
            o << name << ": length = " << buttonLength;
            StartTest(o.str().c_str());
            util::thread::RunInMainAndWait(std::bind(&gui::timeline::DetailsClip::handleLengthButtonPressed,DetailsClipView(),button));
            ASSERT_EQUALS(getSelectedClipsCount(),2);
            ASSERT(VideoClip(0,2)->getSelected());
            ASSERT_CURRENT_COMMAND_TYPE<::command::Combiner>();
            ASSERT_EQUALS(VideoClip(0,2)->getPerceivedLength(), DetailsClipView()->getLength(button));
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
    };
    {
        StartTest("InTransition");
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyPress('i');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("InTransition", { 1, 2, 3, 4, 5, 6, 7 }, { 0, 8 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("OutTransition");
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyPress('o');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("OutTransition", { 1, 2, 3, 4, 5, 6, 7, 8 }, { 0 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("InOutTransition");
        TimelineLeftClick(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("InOutTransition", { 1, 2, 3, 4, 5, 6, 7 }, { 0, 8 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("OutInTransition");
        TimelineLeftClick(RightCenter(VideoClip(0,2)));
        TimelineKeyPress('n');
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("OutInTransition", { 1, 2, 3, 4, 5, 6, 7, 8 }, { 0 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("In+Out Transitions");
        TimelineLeftClick(RightCenter(VideoClip(0,1)));
        TimelineKeyPress('n');
        TimelineLeftClick(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("In+Out Transitions", { 2, 3, 4, 5, 6, 7 }, { 0, 1, 8 });
        Undo(2); // Remove transitions
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_CURRENT_COMMAND_TYPE<::command::ProjectViewCreateSequence>();
    }
    {
        StartTest("InOut+OutIn Transitions");
        TimelineLeftClick(RightCenter(VideoClip(0,1)));
        TimelineKeyPress('n');
        TimelineLeftClick(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
        ASSERT_DETAILSCLIP(VideoClip(0,2));
        pressLengthButtons("InOut+OutIn Transitions", { 2, 3, 4, 5, 6, 7 }, { 0, 1, 8 });
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
            boost::rational<int>{80, 100},
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
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0, 3), model::VideoScalingCustom, rational{ 70,100 }, model::VideoAlignmentCenter, wxPoint(-88, 36), oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Slider: If moved to the left, the scaling is decreased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSlider(), 9000); // Same as presing WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0, 3), model::VideoScalingCustom, rational{ 90,100 }, model::VideoAlignmentCenter, wxPoint(-216, -36), oldRotation);
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
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0, 3), model::VideoScalingCustom, rational{ 81,100 }, model::VideoAlignmentCenter, wxPoint(-158, -3), oldRotation); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved down, the scaling is decreased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSpin(), DetailsClipView()->getScalingSpin()->GetValue() - 0.01); // Same as pressing WXK_DOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0, 3), model::VideoScalingCustom, rational{ 79,100 }, model::VideoAlignmentCenter, wxPoint(-145, 4), oldRotation);// The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit all'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingFitAll);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0, 3), model::VideoScalingFitAll, rational{ 9, 16 }, model::VideoAlignmentCenter, wxPoint(0, 85), oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit to fill'");
        SetValue(DetailsClipView()->getScalingSlider(), 9000); // Same as presing WXK_PAGEDOWN. First change the position a bit.
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingFitToFill);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0, 3), model::VideoScalingFitToFill, rational{ 4,5 }, model::VideoAlignmentCenter, wxPoint(-152, 0), oldRotation);
        Undo(2);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Original size'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingNone);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingNone, 1,model::VideoAlignmentCenter,wxPoint(-280,-72),oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Custom'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingCustom);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingCustom,oldScalingFactor,oldAlignment,oldPosition,oldRotation);
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10),oldRotation);
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenter);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenter,wxPoint(-152,0),oldRotation);
        Undo(3);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center Horizontal'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10),oldRotation);
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterHorizontal);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterHorizontal,wxPoint(-152,10),oldRotation);
        Undo(3);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center Vertical'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>();
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCustom,wxPoint(-142,10),oldRotation);
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterVertical);
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),model::VideoScalingFitToFill,oldScalingFactor,model::VideoAlignmentCenterVertical,wxPoint(-142,0),oldRotation);
        Undo(3);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Slider: moved to the right.");
        SetValue(DetailsClipView()->getRotationSlider(), -10); // Same as WXK_PAGEUP
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(-1,10));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Slider: moved to the left.");
        SetValue(DetailsClipView()->getRotationSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(1,10));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Spin: moved up.");
        SetValue(DetailsClipView()->getRotationSlider(), 1); // Same as WXK_UP
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(1,100));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: moved down.");
        SetValue(DetailsClipView()->getRotationSlider(), -1); // Same as WXK_DOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,3),oldScaling,oldScalingFactor,oldAlignment,oldPosition,boost::rational<int>(-1,100));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Position cursor on center of clip, if the cursor was outside the clip's timeline region");
        TimelinePositionCursor(HCenter(VideoClip(0,1)));
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT_DETAILSCLIP(VideoClip(0,1));
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_DETAILSCLIP(VideoClip(0,3));
        SetValue(DetailsClipView()->getPositionYSlider(),10); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(CursorPosition(),HCenter(VideoClip(0,3))); // Now the cursor is moved to the center of the adjusted clip (for the preview)
        Undo(2);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Keep cursor position, if the cursor was inside the clip's timeline region");
        pixel pos = HCenter(VideoClip(0,3)) - 20;
        TimelinePositionCursor(pos);
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT_DETAILSCLIP(VideoClip(0,1));
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_DETAILSCLIP(VideoClip(0,3));
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(CursorPosition(),pos); // Now the cursor is not moved: same frame is previewed
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
}

void TestDetailsClip::testChangeVolume()
{
    StartTestSuite();

    model::AudioCompositionParameters parameters = model::AudioCompositionParameters().setNrChannels(2).setSampleRate(44100).setPts(AudioClip(0, 3)->getLeftPts()).determineChunkSize();

    model::AudioChunkPtr referenceChunk;
    {
        model::AudioClipPtr audioclip = getAudioClip(AudioClip(0, 3));
        ASSERT_EQUALS(getAudioClip(AudioClip(0, 3))->getVolume(), 100);
        audioclip->moveTo(10);
        referenceChunk = audioclip->getNextAudio(parameters);
        ASSERT_EQUALS(audioclip->getVolume(), 100);
    }

    auto ASSERT_VOLUME = [parameters](model::AudioClipPtr audioclip, int32_t volume, model::AudioChunkPtr referenceChunk)
    {
        ASSERT_EQUALS(audioclip->getVolume(),volume);
        audioclip->moveTo(10);
        model::AudioChunkPtr audiochunk = audioclip->getNextAudio(parameters);

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
        SetValue(DetailsClipView()->getVolumeSlider(),90); // Same as pressing PageUp
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),90,referenceChunk);
        Undo();
        ASSERT_EQUALS(getAudioClip(AudioClip(0,3))->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
    {
        StartTest("Volume: Down via spin");
        SetValue(DetailsClipView()->getVolumeSpin(),97);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),97,referenceChunk);
        Undo();
        ASSERT_EQUALS(getAudioClip(AudioClip(0,3))->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
    {
        StartTest("Volume: Up via slider");
        SetValue(DetailsClipView()->getVolumeSlider(),110);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),110,referenceChunk);
        Undo();
        ASSERT_EQUALS(getAudioClip(AudioClip(0,3))->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
    {
        StartTest("Volume: Up via spin");
        SetValue(DetailsClipView()->getVolumeSpin(),105);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),105,referenceChunk);
        Undo();
        ASSERT_EQUALS(getAudioClip(AudioClip(0,3))->getVolume(), model::Constants::sDefaultVolume);
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
    {
        StartTest("Volume: Change another clip"); // Bug: second edit caused first clip's volume to be changed

        // Edit first clip, don't undo (ensure that this edit action is the most recent executed command)
        SetValue(DetailsClipView()->getVolumeSpin(),85);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),85,referenceChunk);
        // Not: Undo(); -- Keep the current volume command in DetailsClip

        // Get reference data for second edit
        ASSERT_EQUALS(getAudioClip(AudioClip(0,4))->getVolume(),100);
        model::AudioClipPtr audioclip = getAudioClip(AudioClip(0,4));
        audioclip->moveTo(10);
        referenceChunk = audioclip->getNextAudio(parameters);
        ASSERT_VOLUME(getAudioClip(AudioClip(0,4)),100,referenceChunk);

        TimelineLeftClick(Center(AudioClip(0, 4)));
        SetValue(DetailsClipView()->getVolumeSlider(), 70);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,4)),70,referenceChunk);
        Undo();
        ASSERT_VOLUME(getAudioClip(AudioClip(0,4)),model::Constants::sDefaultVolume,referenceChunk);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::EditClipDetails>(); // Previous change audio volume
        Undo();
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
}

void TestDetailsClip::testEditClipDuringPlayback()
{
    StartTestSuite();
    TimelinePositionCursor(HCenter(VideoClip(0,2)));
    TimelineLeftClick(Center(VideoClip(0, 2)));
    {
        StartTest("Edit video clip");
        WaitForPlayback started(true);
        WaitForPlayback stopped(false);
        // NOTE: Don't use WaitForIdle() when the video is playing!!!
        TimelineKeyPress(WXK_SPACE);
        started.wait();
        SetValue(DetailsClipView()->getScalingSlider(), 7000); // Same as presing WXK_PAGEUP
        pause(400);
        util::thread::RunInMain([] { getTimeline().getPlayer()->stop(); });
        stopped.wait();
    }
    {
        StartTest("Edit audio clip");
        WaitForPlayback started(true);
        WaitForPlayback stopped(false);
        // NOTE: Don't use WaitForIdle() when the video is playing!!!
        TimelineKeyPress(WXK_SPACE);
        started.wait();
        SetValue(DetailsClipView()->getVolumeSlider(), 90); // Same as pressing PageUp
        pause(400);
        util::thread::RunInMain([] { getTimeline().getPlayer()->stop(); });
        stopped.wait();
    }
}

void TestDetailsClip::testClipUpdatedAfterDragAndDrop()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_SHOW_PROJECT);
    TimelineDragToTrack(1, VideoClip(0, 3), AudioClip(0, 3));
    TimelineDeleteClips({ VideoClip(0, 0), VideoClip(0, 2), VideoClip(0, 4), VideoClip(0, 5), VideoClip(0,6) });
    TimelineSelectClips({ VideoClip(1,1) });
    // Clip has length of 10s, all other buttons are enabled (10s button is disabled because current length == 10s)
    for (int index : { 0, 1, 2, 3, 4, 5, 6, 7 })
    {
        wxToggleButton* button = DetailsClipView()->getLengthButtons()[index];
        ASSERT(button->IsEnabled());
    }
    TimelineDrag(From(Center(VideoClip(1, 1))).AlignLeft(LeftPixel(VideoClip(0, 1))));
    // Now the clip is updated after the drop. All clips have become disabled. 
    // Clip can't be shortened anymore due to the clip in the other track
    for (int index : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
    {
        wxToggleButton* button = DetailsClipView()->getLengthButtons()[index];
        ASSERT(!button->IsEnabled());
    }
}

} // namespace
