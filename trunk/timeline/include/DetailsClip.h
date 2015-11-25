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

#pragma once

#include "DetailsPanel.h"
#include "Enums.h"
#include "UtilEnumSelector.h"
#include "UtilInt.h"

namespace model {
    class EventChangeAudioClipVolume;
    class EventChangeVideoClipAlignment;
    class EventChangeVideoClipMaxPosition;
    class EventChangeVideoClipMinPosition;
    class EventChangeVideoClipOpacity;
    class EventChangeVideoClipPosition;
    class EventChangeVideoClipRotation;
    class EventChangeVideoClipScaling;
    class EventChangeVideoClipScalingFactor;
    class EventTransitionParameterChanged;
    class EventTransitionParameterChanging;
}

namespace gui { namespace timeline {
    class EventSelectionUpdate;

namespace command {
    class EditClipDetails;
    class EditClipSpeed;
    class TrimClip;
}

class DetailsClip
    : public DetailsPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsClip(wxWindow* parent, Timeline& timeline);
    virtual ~DetailsClip();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    static int factorToSliderValue(rational speed);
    static rational sliderValueToFactor(int slidervalue);

    model::IClipPtr getClip() const;
    void setClip(const model::IClipPtr& clip);

    pts getLength(wxToggleButton* button) const;

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onShow(wxShowEvent& event);
    void onLengthButtonPressed(wxCommandEvent& event);
    void onSpeedSliderChanged(wxCommandEvent& event);
    void onSpeedSpinChanged(wxSpinDoubleEvent& event);

    void onOpacitySliderChanged(wxCommandEvent& event);
    void onOpacitySpinChanged(wxSpinEvent& event);
    void onScalingChoiceChanged(wxCommandEvent& event);
    void onScalingSliderChanged(wxCommandEvent& event);
    void onScalingSpinChanged(wxSpinDoubleEvent& event);
    void onRotationSliderChanged(wxCommandEvent& event);
    void onRotationSpinChanged(wxSpinDoubleEvent& event);
    void onAlignmentChoiceChanged(wxCommandEvent& event);
    void onPositionXSliderChanged(wxCommandEvent& event);
    void onPositionXSpinChanged(wxSpinEvent& event);
    void onPositionYSliderChanged(wxCommandEvent& event);
    void onPositionYSpinChanged(wxSpinEvent& event);

    void onVolumeSliderChanged(wxCommandEvent& event);
    void onVolumeSpinChanged(wxSpinEvent& event);

    // Triggered from a keyboard action in the timeline
    void onTimelineKey(int keycode);

    // Separated for testability: receiving the user action and handling it.
    void handleLengthButtonPressed(wxToggleButton* button);

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpacityChanged(model::EventChangeVideoClipOpacity& event);
    void onScalingChanged(model::EventChangeVideoClipScaling& event);
    void onScalingFactorChanged(model::EventChangeVideoClipScalingFactor& event);
    void onRotationChanged(model::EventChangeVideoClipRotation& event);
    void onAlignmentChanged(model::EventChangeVideoClipAlignment& event);
    void onPositionChanged(model::EventChangeVideoClipPosition& event);
    void onMinPositionChanged(model::EventChangeVideoClipMinPosition& event);
    void onMaxPositionChanged(model::EventChangeVideoClipMaxPosition& event);

    void onVolumeChanged(model::EventChangeAudioClipVolume& event);

    void onTransitionParameterChanged(model::EventTransitionParameterChanged& event);

    //////////////////////////////////////////////////////////////////////////
    // SELECTION EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSelectionChanged(timeline::EventSelectionUpdate& event);

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    std::vector<wxToggleButton*> getLengthButtons() const;
    wxSlider* getSpeedSlider() const;
    wxSpinCtrlDouble* getSpeedSpin() const;

    wxSlider* getOpacitySlider() const;
    wxSpinCtrl* getOpacitySpin() const;
    EnumSelector<model::VideoScaling>* getScalingSelector() const;
    wxSlider* getScalingSlider() const;
    wxSpinCtrlDouble* getScalingSpin() const;
    wxSlider* getRotationSlider() const;
    wxSpinCtrlDouble* getRotationSpin() const;
    EnumSelector<model::VideoAlignment>* getAlignmentSelector() const;
    wxSlider* getPositionXSlider() const;
    wxSpinCtrl* getPositionXSpin() const;
    wxSlider* getPositionYSlider() const;
    wxSpinCtrl* getPositionYSpin() const;

    wxSlider* getVolumeSlider() const;
    wxSpinCtrl* getVolumeSpin() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    struct ClonesContainer
    {
        ClonesContainer(const ClonesContainer& other) = delete;

        explicit ClonesContainer(DetailsClip* details, model::IClipPtr clip);
        virtual ~ClonesContainer();

        DetailsClip* mDetails = nullptr;
        model::IClipPtr Clip = nullptr;
        model::IClipPtr Link = nullptr;
        model::VideoClipPtr Video = nullptr;
        model::AudioClipPtr Audio = nullptr;
    };

    model::IClipPtr mClip = nullptr;      ///< The clip for which the details view is shown. 0 in case a transition is selected
    pts mClipPosition = 0; ///< The currently known position of the clip (used to 'reset' the clip when it is moved around)
    model::TransitionPtr mTransitionClone = nullptr; ///< Transition which is currently being edited
    std::unique_ptr<ClonesContainer> mClones;

    command::EditClipDetails* mEditCommand = nullptr;
    command::EditClipSpeed* mEditSpeedCommand = nullptr;

    wxPanel* mLengthPanel = nullptr;
    std::vector<pts> mLengths;
    std::vector<wxToggleButton*> mLengthButtons;
    std::map<pts, pts> mTrimAtBegin;
    std::map<pts, pts> mTrimAtEnd;

    wxPanel*  mSpeedPanel = nullptr;
    wxSpinCtrlDouble* mSpeedSpin = nullptr;
    wxSlider* mSpeedSlider = nullptr;

    wxPanel* mOpacityPanel = nullptr;
    wxSpinCtrl* mOpacitySpin = nullptr;
    wxSlider* mOpacitySlider = nullptr;

    wxPanel* mRotationPanel = nullptr;
    wxSpinCtrlDouble* mRotationSpin = nullptr;
    wxSlider* mRotationSlider = nullptr;

    wxPanel* mScalingPanel = nullptr;
    EnumSelector<model::VideoScaling>* mSelectScaling = nullptr;
    wxSpinCtrlDouble* mScalingSpin = nullptr;
    wxSlider* mScalingSlider = nullptr;

    wxPanel* mAlignmentPanel = nullptr;
    EnumSelector<model::VideoAlignment>* mSelectAlignment = nullptr;
    wxSpinCtrl* mPositionXSpin = nullptr;
    wxSlider* mPositionXSlider = nullptr;
    wxSpinCtrl* mPositionYSpin = nullptr;
    wxSlider* mPositionYSlider = nullptr;

    pts mMinimumLengthWhenBeginTrimming = 0;
    pts mMaximumLengthWhenBeginTrimming = 0;
    pts mMinimumLengthWhenEndTrimming = 0;
    pts mMaximumLengthWhenEndTrimming = 0;
    pts mMinimumLengthWhenBothTrimming = 0;
    pts mMaximumLengthWhenBothTrimming = 0;

    wxPanel* mVolumePanel = nullptr;
    wxSpinCtrl* mVolumeSpin = nullptr;
    wxSlider* mVolumeSlider = nullptr;

    wxFlexGridSizer* mTransitionBoxSizer = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void submitEditCommandUponAudioVideoEdit(const wxString& message);
    void submitEditCommandUponTransitionEdit(const wxString& parameter);
    void createOrUpdateSpeedCommand(boost::rational<int> speed);

    void preview();

    /// When a slider or spin control is changed for one of the position values, then update
    /// the alignment choice accordingly.
    void updateAlignment(bool horizontalchange);

    void determineClipSizeBounds();

    void updateLengthButtons();
};

}} // namespace
