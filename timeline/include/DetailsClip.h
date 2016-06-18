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

#pragma once

#include "DetailsPanel.h"
#include "Enums.h"
#include "UtilEnumSelector.h"

namespace model {
    class EventChangeAudioClipVolume;
    class EventChangeVideoClipAlignment;
    class EventChangeVideoClipMaxPosition;
    class EventChangeVideoClipMinPosition;
    class EventChangeVideoClipOpacity;
    class EventChangeVideoClipCropTop;
    class EventChangeVideoClipCropBottom;
    class EventChangeVideoClipCropLeft;
    class EventChangeVideoClipCropRight;
    class EventChangeVideoClipPosition;
    class EventChangeVideoClipRotation;
    class EventChangeVideoClipScaling;
    class EventChangeVideoClipScalingFactor;
    class EventTransitionParameterChanged;
    class EventTransitionParameterChanging;
    typedef std::map<pts, VideoKeyFramePtr> VideoKeyFrameMap;
    typedef std::map<pts, AudioKeyFramePtr> AudioeyFrameMap;
}

namespace gui {
    class PlaybackPositionEvent;

    namespace timeline {
    class EventSelectionUpdate;

namespace cmd {
    class EditClipDetails;
    class EditClipSpeed;
    class TrimClip;
}

struct Cleanup;

struct KeyFrameControls;
template <typename CLIPTYPE, typename KEYFRAMETYPE>
struct KeyFrameControlsImpl;

class DetailsClip
    : public DetailsPanel
{
public:

    static constexpr int sRotationPrecision = 2;
    static const int sRotationPrecisionFactor;
    static const int sRotationPageSize;
    static const int sRotationMinNoKeyFrames;
    static const int sRotationMaxNoKeyFrames;
    static const int sRotationMinKeyFrames;
    static const int sRotationMaxKeyFrames;

    static constexpr int sCropPageSize = 10;

    static constexpr int sFactorPrecision = 2;
    static const int sFactorPrecisionFactor;
    static const int sFactorPageSize;
    static const rational64 sFactorMin;
    static const rational64 sFactorMax;

    const wxString sVideo;
    const wxString sAudio;
    const wxString sTransition;

    const wxString sEditOpacity;
    const wxString sEditCropTop;
    const wxString sEditCropBottom;
    const wxString sEditCropLeft;
    const wxString sEditCropRight;
    const wxString sEditScalingType;
    const wxString sEditScaling;
    const wxString sEditRotation;
    const wxString sEditAlignment;
    const wxString sEditX;
    const wxString sEditY;
    const wxString sEditVolume;
    const wxString sEditKeyFramesAdd;
    const wxString sEditKeyFramesRemove;

    static int factorToSliderValue(rational64 speed);
    static rational64 sliderValueToFactor(int slidervalue);

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

    pts getLength(wxToggleButton* button) const;

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onShow(wxShowEvent& event);
    void onSize(wxSizeEvent& event);
    void onLengthButtonPressed(wxCommandEvent& event);
    void onSpeedSliderChanged(wxCommandEvent& event);
    void onSpeedSpinChanged(wxSpinDoubleEvent& event);

    void onPlayButtonPressed(wxCommandEvent& event);
    void onAutoPlayToggled(wxCommandEvent& event);

    void onTransitionType(wxCommandEvent& event);

    void onOpacitySliderChanged(wxCommandEvent& event);
    void onOpacitySpinChanged(wxSpinEvent& event);
    void onCropTopSliderChanged(wxCommandEvent& event);
    void onCropTopSpinChanged(wxSpinEvent& event);
    void onCropBottomSliderChanged(wxCommandEvent& event);
    void onCropBottomSpinChanged(wxSpinEvent& event);
    void onCropLeftSliderChanged(wxCommandEvent& event);
    void onCropLeftSpinChanged(wxSpinEvent& event);
    void onCropRightSliderChanged(wxCommandEvent& event);
    void onCropRightSpinChanged(wxSpinEvent& event);
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

    void onTransitionParameterChanged(model::EventTransitionParameterChanged& event);

    void onSelectionChanged(timeline::EventSelectionUpdate& event);
    void onPlaybackPosition(PlaybackPositionEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    std::vector<wxToggleButton*> getLengthButtons() const;
    wxSlider* getSpeedSlider() const;
    wxSpinCtrlDouble* getSpeedSpin() const;

    wxToggleButton* getPlayButton() const;

    model::TransitionPtr getTransitionClone() const;
    wxChoice* getTransitionTypeSelector() const;

    wxSlider* getOpacitySlider() const;
    wxSpinCtrl* getOpacitySpin() const;
    wxSlider* getCropTopSlider() const;
    wxSpinCtrl* getCropTopSpin() const;
    wxSlider* getCropBottomSlider() const;
    wxSpinCtrl* getCropBottomSpin() const;
    wxSlider* getCropLeftSlider() const;
    wxSpinCtrl* getCropLeftSpin() const;
    wxSlider* getCropRightSlider() const;
    wxSpinCtrl* getCropRightSpin() const;
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

    wxButton* getVideoKeyFramesHomeButton() const;
    wxButton* getVideoKeyFramesPrevButton() const;
    wxButton* getVideoKeyFramesNextButton() const;
    wxButton* getVideoKeyFramesEndButton() const;
    wxButton* getVideoKeyFramesAddButton() const;
    wxButton* getVideoKeyFramesRemoveButton() const;
    size_t getVideoKeyFrameButtonCount() const;
    wxToggleButton* getVideoKeyFrameButton(size_t index) const;

    wxSlider* getVolumeSlider() const;
    wxSpinCtrl* getVolumeSpin() const;

    wxButton* getAudioKeyFramesHomeButton() const;
    wxButton* getAudioKeyFramesPrevButton() const;
    wxButton* getAudioKeyFramesNextButton() const;
    wxButton* getAudioKeyFramesEndButton() const;
    wxButton* getAudioKeyFramesAddButton() const;
    wxButton* getAudioKeyFramesRemoveButton() const;
    size_t getAudioKeyFrameButtonCount() const;
    wxToggleButton* getAudioKeyFrameButton(size_t index) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip = nullptr;      ///< The clip for which the details view is shown. 0 in case a transition is selected
    pts mClipPosition = 0; ///< The currently known position of the clip (used to 'reset' the clip when it is moved around)
    model::TransitionPtr mTransitionClone = nullptr; ///< Transition which is currently being edited

    cmd::EditClipDetails* mEditCommand = nullptr;
    cmd::EditClipSpeed* mEditSpeedCommand = nullptr;
    pts mLastEditKeyFrameOffset = -1;

    std::vector<wxStaticBitmap*> mBitmaps;

    wxPanel* mLengthPanel = nullptr;
    wxStaticBitmap* mBitmapLength = nullptr;
    std::vector<pts> mLengths;
    std::vector<wxToggleButton*> mLengthButtons;
    std::map<pts, pts> mTrimAtBegin;
    std::map<pts, pts> mTrimAtEnd;

    wxPanel* mPlaybackPanel = nullptr;
    wxStaticBitmap* mBitmapPlayback = nullptr;
    wxToggleButton* mPlayButton = nullptr;
    wxCheckBox* mAutoPlayButton = nullptr;
    std::pair<int, int> mPlaybackClipIndex;

    wxPanel* mTransitionTypePanel = nullptr;
    wxChoice* mTransitionType = nullptr;

    wxPanel*  mSpeedPanel = nullptr;
    wxStaticBitmap* mBitmapSpeed = nullptr;
    wxSpinCtrlDouble* mSpeedSpin = nullptr;
    wxSlider* mSpeedSlider = nullptr;

    wxPanel* mOpacityPanel = nullptr;
    wxStaticBitmap* mBitmapOpacity = nullptr;
    wxSpinCtrl* mOpacitySpin = nullptr;
    wxSlider* mOpacitySlider = nullptr;

    wxPanel* mCropPanel = nullptr;
    wxStaticBitmap* mBitmapCropTop = nullptr;
    wxSpinCtrl* mCropTopSpin = nullptr;
    wxSlider* mCropTopSlider = nullptr;
    wxStaticBitmap* mBitmapCropBottom = nullptr;
    wxSpinCtrl* mCropBottomSpin = nullptr;
    wxSlider* mCropBottomSlider = nullptr;
    wxStaticBitmap* mBitmapCropLeft = nullptr;
    wxSpinCtrl* mCropLeftSpin = nullptr;
    wxSlider* mCropLeftSlider = nullptr;
    wxStaticBitmap* mBitmapCropRight = nullptr;
    wxSpinCtrl* mCropRightSpin = nullptr;
    wxSlider* mCropRightSlider = nullptr;

    wxPanel* mRotationPanel = nullptr;
    wxStaticBitmap* mBitmapRotation = nullptr;
    wxSpinCtrlDouble* mRotationSpin = nullptr;
    wxSlider* mRotationSlider = nullptr;

    wxPanel* mScalingPanel = nullptr;
    wxStaticBitmap* mBitmapScaling = nullptr;
    EnumSelector<model::VideoScaling>* mSelectScaling = nullptr;
    wxSpinCtrlDouble* mScalingSpin = nullptr;
    wxSlider* mScalingSlider = nullptr;

    wxPanel* mAlignmentPanel = nullptr;
    wxStaticBitmap* mBitmapAlignment = nullptr;
    wxStaticBitmap* mBitmapX = nullptr;
    wxStaticBitmap* mBitmapY = nullptr;
    EnumSelector<model::VideoAlignment>* mSelectAlignment = nullptr;
    wxSpinCtrl* mPositionXSpin = nullptr;
    wxSlider* mPositionXSlider = nullptr;
    wxSpinCtrl* mPositionYSpin = nullptr;
    wxSlider* mPositionYSlider = nullptr;

    std::shared_ptr<KeyFrameControlsImpl<model::VideoClip, model::VideoKeyFrame>> mVideoKeyFrameControls;

    pts mMinimumLengthWhenBeginTrimming = 0;
    pts mMaximumLengthWhenBeginTrimming = 0;
    pts mMinimumLengthWhenEndTrimming = 0;
    pts mMaximumLengthWhenEndTrimming = 0;
    pts mMinimumLengthWhenBothTrimming = 0;
    pts mMaximumLengthWhenBothTrimming = 0;

    wxPanel* mVolumePanel = nullptr;
    wxStaticBitmap* mBitmapVolume = nullptr;
    wxSpinCtrl* mVolumeSpin = nullptr;
    wxSlider* mVolumeSlider = nullptr;

    std::shared_ptr<KeyFrameControlsImpl<model::AudioClip, model::AudioKeyFrame>> mAudioKeyFrameControls;

    wxPanel* mTransitionPanel = nullptr;
    wxStaticBitmap* mBitmapTransitionType = nullptr;
    std::vector<wxStaticBitmap*> mBitmapsTransitionParameters;
    std::vector<wxStaticText*> mTitlesTransitionParameters;
    wxFlexGridSizer* mTransitionBoxSizer = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void requestShowAndUpdateTitle();

    template <typename, typename> friend struct KeyFrameControlsImpl;

    void submitEditCommandUponAudioVideoEdit(const wxString& message, bool video, std::function<void()> edit);
    void submitEditCommandUponTransitionEdit(const wxString& parameter);
    void submitEditCommandUponTransitionTypeChange(model::TransitionPtr transition);
    void createOrUpdateSpeedCommand(rational64 speed);

    void startStopPlayback(bool ignoreIndex = false);
    void preview();

    /// When a slider or spin control is changed for one of the position values, then update
    /// the alignment choice accordingly.
    void updateAlignment(bool horizontalchange);

    void determineClipSizeBounds();

    void updateLengthButtons();

    void updateSpeedControls();

    void updateIcons() override;

    void updateTitles() override;

    int getNumberOfColumns() const;

    void makeTransitionCloneAndCreateTransitionParameterWidgets(model::IClipPtr clip);
    void createTransitionParameterWidgets();
    void destroyTransitionParameterWidgets();
    std::map<int, model::TransitionPtr> getPossibleVideoTransitions() const;
};

}} // namespace
