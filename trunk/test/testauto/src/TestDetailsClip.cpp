// Copyright 2013-2016 Eric Raijmakers.
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
    ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(rational64{ 4,5 }).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -152,0 }).Rotation(0));
    ASSERT(DetailsView(VideoClip(0, 3)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(rational64{ 4,5 }).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -152,0 }).Rotation(0));
    ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    ASSERT_SELECTION_SIZE(1); // Clip and link selected
    pts originalLength = VideoClip(0, 3)->getLength();

    auto pressLengthButton = [&](wxToggleButton* button, bool enlarge, bool begin) -> int
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
            ASSERT_HISTORY_END(::cmd::Combiner);
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
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
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
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
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
        ASSERT_HISTORY_END(::cmd::Combiner);
        ASSERT_EQUALS(VideoClip(0, 1)->getLength(), buttonlength); // Note: video clip has the same length, audio clip is smaller
        Undo();
    }
}

void TestDetailsClip::testChangeLengthOfTransition()
{
    StartTestSuite();
    TimelineZoomIn(6);
    WindowTriggerMenu(ID_SHOW_PROJECT);

    auto selectClipAndPressLengthButtons = [&] (std::vector<int> enabledButtons, std::vector<int> disabledButtons)
    {
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2))); // Select transition
        ASSERT(DetailsView(VideoClip(0,2)));
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
            ASSERT_HISTORY_END(::cmd::Combiner);
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
        TimelineSelectClips({});
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

    auto pressLengthButtons = [&] (std::string name, std::vector<int> enabledButtons, std::vector<int> disabledButtons)
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
            ASSERT_HISTORY_END(::cmd::Combiner);
            ASSERT_EQUALS(VideoClip(0,2)->getPerceivedLength(), DetailsClipView()->getLength(button));
            Undo(); // Undo: adjust length. Note: Undoing here also revealed a bug here, when the 'TrimClip::doExtraAfter -> change selection' caused 'no selection changed update'
        }
    };
    {
        StartTest("InTransition");
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyPress('i');
        ASSERT(DetailsView(VideoClip(0,1)));
        TimelineSelectClips({ VideoClip(0,2) });
        pressLengthButtons("InTransition", { 1, 2, 3, 4, 5, 6, 7 }, { 0, 8 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_HISTORY_END(::cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("OutTransition");
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyPress('o');
        ASSERT(DetailsView(VideoClip(0,3)));
        TimelineSelectClips({ VideoClip(0,2) });
        pressLengthButtons("OutTransition", { 1, 2, 3, 4, 5, 6, 7, 8 }, { 0 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_HISTORY_END(::cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("InOutTransition");
        TimelineLeftClick(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT(DetailsView(VideoClip(0, 1)));
        TimelineSelectClips({ VideoClip(0,2) });
        ASSERT(DetailsView(VideoClip(0, 2)));
        pressLengthButtons("InOutTransition", { 1, 2, 3, 4, 5, 6, 7 }, { 0, 8 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_HISTORY_END(::cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("OutInTransition");
        TimelineLeftClick(RightCenter(VideoClip(0,2)));
        TimelineKeyPress('n');
        ASSERT(DetailsView(VideoClip(0, 3)));
        TimelineSelectClips({ VideoClip(0,2) });
        ASSERT(DetailsView(VideoClip(0, 2)));
        pressLengthButtons("OutInTransition", { 1, 2, 3, 4, 5, 6, 7, 8 }, { 0 });
        Undo(); // Remove transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_HISTORY_END(::cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("In+Out Transitions");
        TimelineLeftClick(RightCenter(VideoClip(0,1)));
        TimelineKeyPress('n');
        TimelineLeftClick(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
        TimelineSelectClips({ VideoClip(0,2) });
        ASSERT(DetailsView(VideoClip(0,2)));
        pressLengthButtons("In+Out Transitions", { 2, 3, 4, 5, 6, 7 }, { 0, 1, 8 });
        Undo(2); // Remove transitions
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_HISTORY_END(::cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("InOut+OutIn Transitions");
        TimelineLeftClick(RightCenter(VideoClip(0,1)));
        TimelineKeyPress('n');
        TimelineLeftClick(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(Transition);
        TimelineSelectClips({ VideoClip(0,2) });
        ASSERT(DetailsView(VideoClip(0, 2)));
        pressLengthButtons("InOut+OutIn Transitions", { 2, 3, 4, 5, 6, 7 }, { 0, 1, 8 });
        Undo(2); // Remove transitions
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_HISTORY_END(::cmd::ProjectViewCreateSequence);
    }
}

void TestDetailsClip::testTransform()
{
    StartTestSuite();

    gui::Window::get().getUiManager().GetPane("Project").Hide();
    gui::Window::get().getUiManager().GetPane("Details").MinSize(wxSize(600,-1));

    auto ASSERT_ORIGINAL_CLIPPROPERTIES = []
    {
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(rational64{ 4,5 }).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -152,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 3)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(rational64{ 4,5 }).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -152,0 }).Rotation(0));
        // verify only one command is added to the history when doing multiple edits.
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    };

    StartTest("If one clip is selected the details view changes accordingly.");
    TimelineLeftClick(Center(VideoClip(0,3)));
    ASSERT(DetailsView(VideoClip(0,3)));
    ASSERT_ORIGINAL_CLIPPROPERTIES();
    ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);

    {
        StartTest("Scaling: Slider: If moved to the right, the scaling is increased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSlider(), 7000); // Same as presing WXK_PAGEUP
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 70,100 }).Position(wxPoint{ -88, 36 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 70,100 }).Position(wxPoint{ -88, 36 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Slider: If moved to the left, the scaling is decreased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSlider(), 9000); // Same as presing WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 90,100 }).Position(wxPoint{ -216, -36 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 90,100 }).Position(wxPoint{ -216, -36 }));
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
        StartTest("Opacity: Spin");
        SetValue(DetailsClipView()->getOpacitySpin(), 128);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Opacity(128));
        ASSERT(DetailsView(VideoClip(0, 3)).Opacity(128));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Opacity: Slider");
        SetValue(DetailsClipView()->getOpacitySlider(), 128);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Opacity(128));
        ASSERT(DetailsView(VideoClip(0, 3)).Opacity(128));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved up, the scaling is increased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSpin(), DetailsClipView()->getScalingSpin()->GetValue() + 0.01); // Same as pressing WXK_UP
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 81,100 }).Position(wxPoint{ -158, -3 }));// The scaling spin buttons increment with 0.01, not 0.0001
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 81, 100 }).Position(wxPoint{ -158, -3 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Spin: If moved down, the scaling is decreased. Scaling enum is changed to custom.");
        SetValue(DetailsClipView()->getScalingSpin(), DetailsClipView()->getScalingSpin()->GetValue() - 0.01); // Same as pressing WXK_DOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 79,100 }).Position(wxPoint{ -145, 4 })); // The scaling spin buttons increment with 0.01, not 0.0001
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 79,100 }).Position(wxPoint{ -145, 4 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit all'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingFitAll);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingFitAll).ScalingFactor(rational64{ 9, 16 }).Position(wxPoint{ 0, 85 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingFitAll).ScalingFactor(rational64{ 9, 16 }).Position(wxPoint{ 0, 85 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Fit to fill'");
        SetValue(DetailsClipView()->getScalingSlider(), 9000); // Same as presing WXK_PAGEDOWN. First change the position a bit.
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingFitToFill);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingFitToFill).ScalingFactor(rational64{ 4, 5 }).Position(wxPoint{ -152, 0 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingFitToFill).ScalingFactor(rational64{ 4, 5 }).Position(wxPoint{ -152, 0 }));
        Undo(2);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Original size'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingNone);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingNone).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -280, -72 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingNone).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -280, -72 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Scaling: Choice: 'Custom'");
        SetValue(DetailsClipView()->getScalingSelector(),model::VideoScalingCustom);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Scaling(model::VideoScalingCustom));
        ASSERT(DetailsView(VideoClip(0, 3)).Scaling(model::VideoScalingCustom));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ -142, 10 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ -142, 10 }));
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenter);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -152, 0 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ -152, 0 }));
        Undo(3);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center Horizontal'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ -142, 10 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ -142, 10 }));
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterHorizontal);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Alignment(model::VideoAlignmentCenterHorizontal).Position(wxPoint{ -152, 10 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Alignment(model::VideoAlignmentCenterHorizontal).Position(wxPoint{ -152, 10 }));
        Undo(3);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Alignment: Choice: 'Center Vertical'");
        // First, set different values
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionYSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ -142, 10 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ -142, 10 }));
        SetValue(DetailsClipView()->getAlignmentSelector(),model::VideoAlignmentCenterVertical);
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Alignment(model::VideoAlignmentCenterVertical).Position(wxPoint{ -142, 0 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Alignment(model::VideoAlignmentCenterVertical).Position(wxPoint{ -142, 0 }));
        Undo(3);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Slider: moved to the right.");
        SetValue(DetailsClipView()->getRotationSlider(), -10); // Same as WXK_PAGEUP
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Rotation(rational64{ -1,10 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Rotation(rational64{ -1,10 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Slider: moved to the left.");
        SetValue(DetailsClipView()->getRotationSlider(), 10); // Same as WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Rotation(rational64{ 1, 10 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Rotation(rational64{ 1, 10 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Spin: moved up.");
        SetValue(DetailsClipView()->getRotationSlider(), 1); // Same as WXK_UP
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Rotation(rational64{ 1, 100 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Rotation(rational64{ 1, 100 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Rotation: Spin: moved down.");
        SetValue(DetailsClipView()->getRotationSlider(), -1); // Same as WXK_DOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Rotation(rational64{ -1, 100 }));
        ASSERT(DetailsView(VideoClip(0, 3)).Rotation(rational64{ -1, 100 }));
        Undo();
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Position cursor on center of clip, if the cursor was outside the clip's timeline region");
        TimelinePositionCursor(HCenter(VideoClip(0,1)));
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT(DetailsView(VideoClip(0,1)));
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT(DetailsView(VideoClip(0, 3)));
        SetValue(DetailsClipView()->getPositionYSlider(),10); // Same as WXK_PAGEDOWN
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(CursorPosition(),HCenter(VideoClip(0,3))); // Now the cursor is moved to the center of the adjusted clip (for the preview)
        Undo(2);
        ASSERT_ORIGINAL_CLIPPROPERTIES();
    }
    {
        StartTest("Keep cursor position, if the cursor was inside the clip's timeline region");
        pixel pos = HCenter(VideoClip(0,3)) - 20;
        TimelinePositionCursor(pos);
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT(DetailsView(VideoClip(0,1)));
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT(DetailsView(VideoClip(0,3)));
        SetValue(DetailsClipView()->getPositionXSlider(), -142); // Same as WXK_PAGEDOWN
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
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
        ASSERT_EQUALS(DefaultAudioKeyFrame(AudioClip(0, 3))->getVolume(), 100);
        audioclip->moveTo(10);
        referenceChunk = audioclip->getNextAudio(parameters);
        ASSERT_EQUALS(DefaultAudioKeyFrame(audioclip)->getVolume(), 100);
    }

    auto ASSERT_VOLUME = [&](model::AudioClipPtr audioclip, int32_t volume, model::AudioChunkPtr referenceChunk)
    {
        ASSERT_EQUALS(DefaultAudioKeyFrame(audioclip)->getVolume(),volume);
        audioclip->moveTo(10);
        model::AudioChunkPtr audiochunk = audioclip->getNextAudio(parameters);

        sample* cur = audiochunk->getUnreadSamples();
        sample* ref = referenceChunk->getUnreadSamples();
        sample* last = cur + 100;
        double v{ volume / 100.0 };
        while (cur < last)
        {
            sample c = *cur;
            sample r{ *ref };
            model::adjustSampleVolume(v, r);
            ASSERT_EQUALS(c,r);
            cur++;
            ref++;
        }
    };

    TimelineLeftClick(Center(AudioClip(0,3)));

    {
        StartTest("Volume: Down via slider");
        SetValue(DetailsClipView()->getVolumeSlider(),90); // Same as pressing PageUp
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),90,referenceChunk);
        Undo();
        ASSERT_EQUALS(DefaultAudioKeyFrame(AudioClip(0,3))->getVolume(), model::AudioKeyFrame::sVolumeDefault);
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("Volume: Down via spin");
        SetValue(DetailsClipView()->getVolumeSpin(),97);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),97,referenceChunk);
        Undo();
        ASSERT_EQUALS(DefaultAudioKeyFrame(AudioClip(0,3))->getVolume(), model::AudioKeyFrame::sVolumeDefault);
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("Volume: Up via slider");
        SetValue(DetailsClipView()->getVolumeSlider(),110);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),110,referenceChunk);
        Undo();
        ASSERT_EQUALS(DefaultAudioKeyFrame(AudioClip(0,3))->getVolume(), model::AudioKeyFrame::sVolumeDefault);
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("Volume: Up via spin");
        SetValue(DetailsClipView()->getVolumeSpin(),105);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),105,referenceChunk);
        Undo();
        ASSERT_EQUALS(DefaultAudioKeyFrame(AudioClip(0,3))->getVolume(), model::AudioKeyFrame::sVolumeDefault);
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    }
    {
        StartTest("Volume: Change another clip"); // Bug: second edit caused first clip's volume to be changed

        // Edit first clip, don't undo (ensure that this edit action is the most recent executed command)
        SetValue(DetailsClipView()->getVolumeSpin(),85);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,3)),85,referenceChunk);
        // Not: Undo(); -- Keep the current volume command in DetailsClip

        // Get reference data for second edit
        ASSERT_EQUALS(DefaultAudioKeyFrame(AudioClip(0,4))->getVolume(),100);
        model::AudioClipPtr audioclip = getAudioClip(AudioClip(0,4));
        audioclip->moveTo(10);
        referenceChunk = audioclip->getNextAudio(parameters);
        ASSERT_VOLUME(getAudioClip(AudioClip(0,4)),100,referenceChunk);

        TimelineLeftClick(Center(AudioClip(0, 4)));
        SetValue(DetailsClipView()->getVolumeSlider(), 70);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_VOLUME(getAudioClip(AudioClip(0,4)),70,referenceChunk);
        Undo();
        ASSERT_VOLUME(getAudioClip(AudioClip(0,4)),model::AudioKeyFrame::sVolumeDefault,referenceChunk);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Previous change audio volume
        Undo();
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
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
