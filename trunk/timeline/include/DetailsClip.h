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

#ifndef DETAILS_CLIP_H
#define DETAILS_CLIP_H

#include "DetailsPanel.h"
#include "Enums.h"
#include "UtilEnumSelector.h"
#include "UtilInt.h"

namespace model {
    class ChangeVideoClipTransform;
    class ChangeAudioClipVolume;
    class EventChangeVideoClipOpacity;
    class EventChangeVideoClipScaling;
    class EventChangeVideoClipScalingFactor;
    class EventChangeVideoClipRotation;
    class EventChangeVideoClipAlignment;
    class EventChangeVideoClipPosition;
    class EventChangeVideoClipMinPosition;
    class EventChangeVideoClipMaxPosition;
    class EventChangeAudioClipVolume;
}

namespace gui { namespace timeline {
    class EventSelectionUpdate;

namespace command {
    class TrimClip;
}

class DetailsClip
:   public DetailsPanel
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

    model::IClipPtr getClip() const;
    void setClip(const model::IClipPtr& clip);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onShow(wxShowEvent& event);
    void onLengthButtonPressed(wxCommandEvent& event);
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

    //////////////////////////////////////////////////////////////////////////
    // SELECTION EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSelectionChanged(timeline::EventSelectionUpdate& event);

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    std::list<wxToggleButton*> getLengthButtons() const;
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

    model::IClipPtr      mClip;      ///< The clip for which the details view is shown. 0 in case a transition is selected
    model::VideoClipPtr  mVideoClip;
    model::AudioClipPtr  mAudioClip;
    model::TransitionPtr mTransition;
    model::EmptyClipPtr  mEmptyClip;

    wxStaticText* mCurrentLength;
    std::list<wxToggleButton*> mLengthButtons;
    std::map<pts, pts> mTrimAtBegin;
    std::map<pts, pts> mTrimAtEnd;

    wxSpinCtrl* mOpacitySpin;
    wxSlider* mOpacitySlider;

    EnumSelector<model::VideoScaling>* mSelectScaling;

    wxSpinCtrlDouble* mScalingSpin;
    wxSlider* mScalingSlider;

    wxSpinCtrlDouble* mRotationSpin;
    wxSlider* mRotationSlider;

    EnumSelector<model::VideoAlignment>* mSelectAlignment;

    wxSpinCtrl* mPositionXSpin;
    wxSlider* mPositionXSlider;
    wxSpinCtrl* mPositionYSpin;
    wxSlider* mPositionYSlider;

    model::ChangeVideoClipTransform* mTransformCommand;
    model::ChangeAudioClipVolume* mVolumeCommand;

    pts mMinimumLengthWhenBeginTrimming;
    pts mMaximumLengthWhenBeginTrimming;
    pts mMinimumLengthWhenEndTrimming;
    pts mMaximumLengthWhenEndTrimming;
    pts mMinimumLengthWhenBothTrimming;
    pts mMaximumLengthWhenBothTrimming;

    wxSpinCtrl* mVolumeSpin;
    wxSlider* mVolumeSlider;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void makeTransformCommand();
    void makeChangeVolumeCommand();

    void preview();

    /// When a slider or spin control is changed for one of the position values, then update
    /// the alignment choice accordingly.
    void updateAlignment(bool horizontalchange);

    void determineClipSizeBounds();

    void updateLengthButtons();
};

}} // namespace

#endif
