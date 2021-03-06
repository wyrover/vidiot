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

#include "DetailsClip.h"

#include "Config.h"
#include "Selection.h"
#include "SelectionEvent.h"
#include "UtilWindow.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

constexpr double sFactorIncrement = 0.01;
constexpr double sRotationIncrement = 0.01;

constexpr int sPositionPageSize = 10;
constexpr int sOpacityPageSize = 10;
constexpr int sVolumePageSize = 10;

const int DetailsClip::sRotationPrecisionFactor = static_cast<int>(pow(10.0, DetailsClip::sRotationPrecision)); ///< 10^sRotationPrecision
const int DetailsClip::sRotationPageSize = DetailsClip::sRotationPrecisionFactor / 10; // 0.1
const int DetailsClip::sRotationMinNoKeyFrames = -180 * DetailsClip::sRotationPrecisionFactor;
const int DetailsClip::sRotationMaxNoKeyFrames = 180 * DetailsClip::sRotationPrecisionFactor;
const int DetailsClip::sRotationMinKeyFrames = -1800 * DetailsClip::sRotationPrecisionFactor;
const int DetailsClip::sRotationMaxKeyFrames = 1800 * DetailsClip::sRotationPrecisionFactor;

const int DetailsClip::sFactorPrecisionFactor = static_cast<int>(pow(10.0, sFactorPrecision)); ///< 10^sFactorPrecision
const int DetailsClip::sFactorPageSize = sFactorPrecision / 10; // 0.1
const rational64 DetailsClip::sFactorMin{ 1,100 }; // 0.01
const rational64 DetailsClip::sFactorMax{ 100,1 }; // 100

DetailsClip::DetailsClip(wxWindow* parent, Timeline& timeline)
    : DetailsPanel(parent, timeline)
    , sVideo(_("Video"))
    , sAudio(_("Audio"))
    , sTransition(_("Transition"))
    , sEditOpacity(_("Edit opacity of %s"))
    , sEditScalingType(_("Edit automated scaling of %s"))
    , sEditScaling(_("Edit scaling of %s"))
    , sEditRotation(_("Edit rotation of %s"))
    , sEditAlignment(_("Edit automated alignment of %s"))
    , sEditX(_("Edit X position of %s"))
    , sEditY(_("Edit Y position of %s"))
    , sEditVolume(_("Edit volume of %s"))
    , sEditKeyFramesAdd(_("Add key frame to %s"))
    , sEditKeyFramesRemove(_("Remove key frame from %s"))
    , mPlaybackClipIndex{ std::make_pair(-1, -1) }
    , mVideoKeyFrameControls(std::make_shared<KeyFrameControlsImpl<model::VideoClip, model::VideoKeyFrame>>(this, _("Add/remove/select video key frames")))
    , mAudioKeyFrameControls(std::make_shared<KeyFrameControlsImpl<model::AudioClip, model::AudioKeyFrame>>(this, _("Add/remove/select audio key frames")))
{
    VAR_DEBUG(this);

    addBox(boost::none, 2);

    {
        // Read lengths from config
        wxString lengthButtons = Config::get().read<wxString>(Config::sPathTimelineLengthButtons);
        wxStringTokenizer t(lengthButtons, ",", wxTOKEN_STRTOK);
        while (t.HasMoreTokens() && mLengths.size() < 9) // Keys/Buttons 1-9 may be customized.
        {
            wxString token(t.GetNextToken());
            int length{ wxAtoi(token) };
            length = std::max(length, 100); // Too small values may cause errors in clip length logic.
            length = std::min(length, 3600000);
            mLengths.emplace_back(length);
            std::sort(mLengths.begin(), mLengths.end());
            auto endIt = std::unique(mLengths.begin(), mLengths.end());
            mLengths.resize( std::distance(mLengths.begin(), endIt) );
        }
        if (mLengths.empty())
        {
            mLengths = { 250, 500, 1000, 1500, 2000, 2500, 3000, 5000, 10000 }; // Keep in sync with defaults in Config. The defaults in config ensure that after startup the setting is present in the file.
            wxString configValue;
            for (int i : mLengths) { configValue << i << ','; }
            Config::get().write<wxString>(Config::sPathTimelineLengthButtons, configValue);
            Config::get().Flush();
        }               
    }

    mLengthPanel = new wxPanel(this);
    mLengthPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    mBitmapLength = new wxStaticBitmap(mLengthPanel, wxID_ANY, util::window::getBitmap("clock-select-blue.png"));
    mBitmapLength->SetToolTip(_("Change the length of the clip to one of the standard lengths. Will shift other clips to avoid introducing a black area."));
    mLengthPanel->GetSizer()->Add(mBitmapLength, wxSizerFlags(0).Left().CenterVertical());
    for (unsigned int i = 0; i < mLengths.size(); ++i)
    {
        std::ostringstream os;
        os << std::setprecision(3) << std::fixed << static_cast<float>(mLengths[i]) / 1000.0;
        wxString label(os.str());
        if (label.EndsWith('0')) { label.RemoveLast(); }
        if (label.EndsWith('0')) { label.RemoveLast(); }

        // Use the integer as id
        wxToggleButton* button = new wxToggleButton(mLengthPanel, i, label, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        button->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
        button->SetToolTip(_("Change the length of the clip to this length. Will shift other clips to avoid introducing a black area.") + " " + _("Shortcut key") + ": '" + wxString::Format("%d", i + 1) + "'");
        mLengthPanel->GetSizer()->Add(button,wxSizerFlags(1));
        button->Bind( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
        mLengthButtons.emplace_back(button);
    }
    updateLengthButtons();
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Fixed lengths (s)"), mLengthPanel, mBitmapLength->GetToolTipText());

    mSpeedPanel = new wxPanel(this);
    wxBoxSizer* speedsizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapSpeed = new wxStaticBitmap(mSpeedPanel, wxID_ANY, util::window::getBitmap("dashboard-blue.png"));
    mSpeedSlider = new wxSlider(mSpeedPanel, wxID_ANY, factorToSliderValue(sFactorMax), factorToSliderValue(sFactorMin), factorToSliderValue(sFactorMax));
    mSpeedSlider->SetPageSize(sFactorPageSize);
    mSpeedSlider->SetToolTip(_("Change the speed of the clip. Changing the clip speed is only allowed when the clip and its linked clip (if any) are both of an equal size and position. Furthermore, there may be no other clips in the same time span as the clip. Note that, when selecting a speed smaller than 0.5 or larger than 2.0 the pitch of the audio will change."));
    mSpeedSpin = new wxSpinCtrlDouble(mSpeedPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mSpeedSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mSpeedSpin->SetDigits(sFactorPrecision);
    mSpeedSpin->SetValue(1);                         
    mSpeedSpin->SetRange(boost::rational_cast<double>(sFactorMin), boost::rational_cast<double>(sFactorMax));
    mSpeedSpin->SetIncrement(sFactorIncrement);
    mSpeedSpin->SetToolTip(mSpeedSlider->GetToolTipText());
    mBitmapSpeed->SetToolTip(mSpeedSlider->GetToolTipText());
    speedsizer->Add(mBitmapSpeed, wxSizerFlags(0).Left().CenterVertical());
    speedsizer->Add(mSpeedSlider, wxSizerFlags(1).Expand());
    speedsizer->Add(mSpeedSpin, wxSizerFlags(0));
    mSpeedPanel->SetSizer(speedsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Speed"), mSpeedPanel, mSpeedSlider->GetToolTipText());

    mSpeedSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onSpeedSliderChanged, this);
    mSpeedSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onSpeedSpinChanged, this);

    mPlaybackPanel = new wxPanel(this);
    wxBoxSizer* playbacksizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapPlayback = new wxStaticBitmap(mPlaybackPanel, wxID_ANY, util::window::getBitmap("arrow-repeat.png"));
    mPlayButton = new wxToggleButton(mPlaybackPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    mPlayButton->SetBitmap(util::window::getIcon("icon-pauseplay.png"), wxTOP);
    mPlayButton->SetToolTip(_("Press this button to start/stop continuous playback of the transition."));
    mPlayButton->SetValue(false);
    mAutoPlayButton = new wxCheckBox(mPlaybackPanel, wxID_ANY, _("Start automatically"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    mAutoPlayButton->SetToolTip(_("Select this to immediately start continuous playback of transition when a transition is selected."));
    mAutoPlayButton->SetValue(Config::get().read<bool>(Config::sPathEditAutoStartPlayback));
    mBitmapPlayback->SetToolTip(_("Start/stop continous playback of the transition."));       
    playbacksizer->Add(mBitmapPlayback, wxSizerFlags(0).Left().CenterVertical());
    playbacksizer->Add(mPlayButton, wxSizerFlags(0));
    playbacksizer->Add(mAutoPlayButton, wxSizerFlags(0).CenterVertical());
    mPlaybackPanel->SetSizer(playbacksizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Preview"), mPlaybackPanel, mBitmapPlayback->GetToolTipText());

    mPlayButton->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onPlayButtonPressed, this);
    mAutoPlayButton->Bind(wxEVT_CHECKBOX, &DetailsClip::onAutoPlayToggled, this);

    mTransitionTypePanel = new wxPanel(this);
    wxBoxSizer* transitiontypesizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapTransitionType = new wxStaticBitmap(mTransitionTypePanel, wxID_ANY, util::window::getBitmap("question-blue.png"));
    mTransitionType = new wxChoice(mTransitionTypePanel, wxID_ANY);
    mTransitionType->SetToolTip(_("Select the effect used for the transition towards the second clip."));
    mBitmapTransitionType->SetToolTip(mTransitionType->GetToolTipText());
    transitiontypesizer->Add(mBitmapTransitionType, wxSizerFlags(0).Left().CenterVertical());
    transitiontypesizer->Add(mTransitionType, wxSizerFlags(1));
    mTransitionTypePanel->SetSizer(transitiontypesizer);     
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Type"), mTransitionTypePanel, mTransitionType->GetToolTipText());

    mTransitionType->Bind(wxEVT_CHOICE, &DetailsClip::onTransitionType, this);

    mOpacityPanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapOpacity = new wxStaticBitmap(mOpacityPanel, wxID_ANY, util::window::getBitmap("gradient-blue2.png"));
    mOpacitySlider = new wxSlider(mOpacityPanel, wxID_ANY, model::VideoKeyFrame::sOpacityMax, model::VideoKeyFrame::sOpacityMin, model::VideoKeyFrame::sOpacityMax );
    mOpacitySlider->SetPageSize(sOpacityPageSize);
    mOpacitySlider->SetToolTip(_("Select the opacity of the video frame. A value of '0' means fully transparent. A value of '255' means fully opaque."));
    mOpacitySpin = new wxSpinCtrl(mOpacityPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mOpacitySpin->SetRange(model::VideoKeyFrame::sOpacityMin, model::VideoKeyFrame::sOpacityMax);
    mOpacitySpin->SetValue(model::VideoKeyFrame::sOpacityMax);
    mOpacitySpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mOpacitySpin->SetToolTip(mOpacitySlider->GetToolTipText());
    mBitmapOpacity->SetToolTip(mOpacitySlider->GetToolTipText());
    opacitysizer->Add(mBitmapOpacity, wxSizerFlags(0).Left().CenterVertical());
    opacitysizer->Add(mOpacitySlider, wxSizerFlags(1).Expand());
    opacitysizer->Add(mOpacitySpin, wxSizerFlags(0));
    mOpacityPanel->SetSizer(opacitysizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Opacity"), mOpacityPanel, mOpacitySlider->GetToolTipText());

    mCropPanel = new wxPanel(this);
    static wxString sCropRemark(_("Note that automated scaling and positioning apply to the cropped (smaller) image. If the image should not be rescaled/repositioned after cropping, select Custom for the automated scaling and the automated positioning."));
    wxBoxSizer* cropsizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapCropTop = new wxStaticBitmap(mCropPanel, wxID_ANY, util::window::getBitmap("arrow-transition-270.png"));
    mCropTopSlider = new wxSlider(mCropPanel, wxID_ANY, 0, 0, 1);
    mCropTopSlider->SetPageSize(sCropPageSize);
    mCropTopSlider->SetToolTip(_("Select the area to remove from the top of the video.") + " " + sCropRemark);
    mBitmapCropTop->SetToolTip(mCropTopSlider->GetToolTipText());
    mCropTopSpin = new wxSpinCtrl(mCropPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
    mCropTopSpin->SetRange(0, 0);
    mCropTopSpin->SetValue(0);
    mCropTopSpin->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    mCropTopSpin->SetToolTip(mCropTopSlider->GetToolTipText());
    mBitmapCropBottom = new wxStaticBitmap(mCropPanel, wxID_ANY, util::window::getBitmap("arrow-transition-090.png"));
    mCropBottomSlider = new wxSlider(mCropPanel, wxID_ANY, 0, 0, 1);
    mCropBottomSlider->SetPageSize(sCropPageSize);
    mCropBottomSlider->SetToolTip(_("Select the area to remove from the bottom of the video.") + " " + sCropRemark);
    mBitmapCropBottom->SetToolTip(mCropBottomSlider->GetToolTipText());
    mCropBottomSpin = new wxSpinCtrl(mCropPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
    mCropBottomSpin->SetRange(0, 0);
    mCropBottomSpin->SetValue(0);
    mCropBottomSpin->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    mCropBottomSpin->SetToolTip(mCropBottomSlider->GetToolTipText());
    mBitmapCropLeft = new wxStaticBitmap(mCropPanel, wxID_ANY, util::window::getBitmap("arrow-transition.png"));
    mCropLeftSlider = new wxSlider(mCropPanel, wxID_ANY, 0, 0, 1);
    mCropLeftSlider->SetPageSize(sCropPageSize);
    mCropLeftSlider->SetToolTip(_("Select the area to remove from the left side of the video.") + " " + sCropRemark);
    mBitmapCropLeft->SetToolTip(mCropLeftSlider->GetToolTipText());
    mCropLeftSpin = new wxSpinCtrl(mCropPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
    mCropLeftSpin->SetRange(0, 0);
    mCropLeftSpin->SetValue(0);
    mCropLeftSpin->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    mCropLeftSpin->SetToolTip(mCropLeftSlider->GetToolTipText());
    mBitmapCropRight = new wxStaticBitmap(mCropPanel, wxID_ANY, util::window::getBitmap("arrow-transition-180.png"));
    mCropRightSlider = new wxSlider(mCropPanel, wxID_ANY, 0, 0, 1);
    mCropRightSlider->SetPageSize(sCropPageSize);
    mCropRightSlider->SetToolTip(_("Select the area to remove from the right side of the video.") + " " + sCropRemark);
    mBitmapCropRight->SetToolTip(mCropRightSlider->GetToolTipText());
    mCropRightSpin = new wxSpinCtrl(mCropPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
    mCropRightSpin->SetRange(0, 0);
    mCropRightSpin->SetValue(0);
    mCropRightSpin->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    mCropRightSpin->SetToolTip(mCropRightSlider->GetToolTipText());
    cropsizer->Add(mBitmapCropTop, wxSizerFlags(0).Left().CenterVertical());
    cropsizer->Add(mCropTopSlider, wxSizerFlags(1).Expand());
    cropsizer->Add(mCropTopSpin, wxSizerFlags(0));
    cropsizer->AddSpacer(4);
    cropsizer->Add(mBitmapCropBottom, wxSizerFlags(0).Left().CenterVertical());
    cropsizer->Add(mCropBottomSlider, wxSizerFlags(1).Expand());
    cropsizer->Add(mCropBottomSpin, wxSizerFlags(0));
    cropsizer->AddSpacer(4);
    cropsizer->Add(mBitmapCropLeft, wxSizerFlags(0).Left().CenterVertical());
    cropsizer->Add(mCropLeftSlider, wxSizerFlags(1).Expand());
    cropsizer->Add(mCropLeftSpin, wxSizerFlags(0));
    cropsizer->AddSpacer(4);
    cropsizer->Add(mBitmapCropRight, wxSizerFlags(0).Left().CenterVertical());
    cropsizer->Add(mCropRightSlider, wxSizerFlags(1).Expand());
    cropsizer->Add(mCropRightSpin, wxSizerFlags(0));
    mCropPanel->SetSizer(cropsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Crop"), mCropPanel, _("Remove unwanted regions from the side of the video."));

    mRotationPanel = new wxPanel(this);
    wxBoxSizer* rotationsizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapRotation = new wxStaticBitmap(mRotationPanel, wxID_ANY, util::window::getBitmap("arrow-circle-double-135.png"));
    mRotationSlider = new wxSlider(mRotationPanel,wxID_ANY, 1 * sRotationPrecisionFactor, sRotationMinNoKeyFrames, sRotationMaxNoKeyFrames);
    mRotationSlider->SetPageSize(sRotationPageSize);
    mRotationSlider->SetToolTip(_("Select the rotation (in degrees) of the video frame."));
    mBitmapRotation->SetToolTip(mRotationSlider->GetToolTipText());
    mRotationSpin = new wxSpinCtrlDouble(mRotationPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mRotationSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mRotationSpin->SetDigits(sRotationPrecision);
    mRotationSpin->SetValue(0); // No rotation
    mRotationSpin->SetRange(
        static_cast<double>(sRotationMinNoKeyFrames) / static_cast<double>(sRotationPrecisionFactor),
        static_cast<double>(sRotationMaxNoKeyFrames) / static_cast<double>(sRotationPrecisionFactor));
    mRotationSpin->SetIncrement(sRotationIncrement);
    mRotationSpin->SetToolTip(mRotationSlider->GetToolTipText());
    rotationsizer->Add(mBitmapRotation, wxSizerFlags(0).Left().CenterVertical());
    rotationsizer->Add(mRotationSlider, wxSizerFlags(1).Expand());
    rotationsizer->Add(mRotationSpin, wxSizerFlags(0));
    mRotationPanel->SetSizer(rotationsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Rotation"), mRotationPanel, mRotationSlider->GetToolTipText());

    mScalingPanel = new wxPanel(this);
    wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapScaling = new wxStaticBitmap(mScalingPanel, wxID_ANY, util::window::getBitmap("arrow-in-out.png"));
    mSelectScaling = new EnumSelector<model::VideoScaling>(mScalingPanel, model::VideoScalingConverter::getMapToHumanReadibleString(), model::VideoScalingNone);
    mSelectScaling->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mSelectScaling->SetToolTip(_("Select between several automated scaling strategies or manually select the (custom) scaling factor of the video frame."));
    mBitmapScaling->SetToolTip(mSelectScaling->GetToolTipText());
    mScalingSlider = new wxSlider(mScalingPanel,wxID_ANY, factorToSliderValue(model::VideoKeyFrame::sScalingMax), factorToSliderValue(model::VideoKeyFrame::sScalingMin), factorToSliderValue(model::VideoKeyFrame::sScalingMax));
    mScalingSlider->SetPageSize(sFactorPageSize);
    mScalingSlider->SetToolTip(_("Select the scaling factor to be applied to the video frame."));
    mScalingSpin = new wxSpinCtrlDouble(mScalingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mScalingSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSpin->SetValue(1); // No scaling
    mScalingSpin->SetDigits(sFactorPrecision);
    mScalingSpin->SetRange(boost::rational_cast<double>(model::VideoKeyFrame::sScalingMin), boost::rational_cast<double>(model::VideoKeyFrame::sScalingMax));
    mScalingSpin->SetIncrement(sFactorIncrement);
    mScalingSpin->SetToolTip(mScalingSlider->GetToolTipText());
    scalingsizer->Add(mBitmapScaling, wxSizerFlags(0).Left().CenterVertical());
    scalingsizer->Add(mSelectScaling, wxSizerFlags(0).Right().CenterVertical());
    scalingsizer->Add(mScalingSlider, wxSizerFlags(1).Expand());
    scalingsizer->Add(mScalingSpin, wxSizerFlags(0));
    mScalingPanel->SetSizer(scalingsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Scaling"), mScalingPanel, mSelectScaling->GetToolTipText());

    mAlignmentPanel = new wxPanel(this);
    wxBoxSizer* alignmentsizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapAlignment = new wxStaticBitmap(mAlignmentPanel, wxID_ANY, util::window::getBitmap("arrow-move.png"));
    mSelectAlignment = new EnumSelector<model::VideoAlignment>(mAlignmentPanel, model::VideoAlignmentConverter::getMapToHumanReadibleString(), model::VideoAlignmentCustom);
    mSelectAlignment->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mSelectAlignment->SetToolTip(_("Select between several automated positioning strategies or manually select the (custom) position of the video frame."));
    mBitmapAlignment->SetToolTip(mSelectAlignment->GetToolTipText());
    mBitmapX = new wxStaticBitmap(mAlignmentPanel, wxID_ANY, util::window::getBitmap("arrow-resize.png"));
    mPositionXSlider = new wxSlider(mAlignmentPanel, wxID_ANY, 0, 0, 1);
    mPositionXSlider->SetPageSize(sPositionPageSize);
    mPositionXSlider->SetToolTip(_("Select the horizontal position of the video frame."));
    mBitmapX->SetToolTip(mPositionXSlider->GetToolTipText());
    mPositionXSpin = new wxSpinCtrl(mAlignmentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionXSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionXSpin->SetRange(0,1);
    mPositionXSpin->SetValue(0);
    mPositionXSpin->SetToolTip(mPositionXSlider->GetToolTipText());
    mBitmapY = new wxStaticBitmap(mAlignmentPanel, wxID_ANY, util::window::getBitmap("arrow-resize-090.png"));
    mPositionYSlider = new wxSlider(mAlignmentPanel, wxID_ANY, 0, 0, 1);
    mPositionYSlider->SetPageSize(sPositionPageSize);
    mPositionYSlider->SetToolTip(_("Select the vertical position of the video frame."));
    mBitmapY->SetToolTip(mPositionYSlider->GetToolTipText());
    mPositionYSpin = new wxSpinCtrl(mAlignmentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionYSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionYSpin->SetRange(0,1);                   
    mPositionYSpin->SetValue(0);
    mPositionYSpin->SetToolTip(mPositionYSlider->GetToolTipText());
    alignmentsizer->Add(mBitmapAlignment, wxSizerFlags(0).Left().CenterVertical());
    alignmentsizer->Add(mSelectAlignment, wxSizerFlags(0).Right().CenterVertical());
    alignmentsizer->AddSpacer(4);
    alignmentsizer->Add(mBitmapX, wxSizerFlags(0).Left().CenterHorizontal().CenterVertical());
    alignmentsizer->Add(mPositionXSlider, wxSizerFlags(1000).Expand());
    alignmentsizer->Add(mPositionXSpin, wxSizerFlags(0).Expand());
    alignmentsizer->AddSpacer(4);
    alignmentsizer->Add(mBitmapY, wxSizerFlags(0).Left().CenterHorizontal().CenterVertical());
    alignmentsizer->Add(mPositionYSlider, wxSizerFlags(1000));
    alignmentsizer->Add(mPositionYSpin, wxSizerFlags(0).Expand());
    mAlignmentPanel->SetSizer(alignmentsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Position"), mAlignmentPanel, mSelectAlignment->GetToolTipText());

    // Give these two the same width
    mSelectAlignment->SetMinSize(wxSize(mSelectScaling->GetSize().x,-1));
    mSelectScaling->SetMinSize(wxSize(mSelectAlignment->GetSize().x,-1));

    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Video key frames"), mVideoKeyFrameControls->mEditPanel, mVideoKeyFrameControls->mToolTip);

    mOpacitySlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onOpacitySliderChanged, this);
    mOpacitySpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onOpacitySpinChanged, this);
    mCropTopSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropTopSliderChanged, this);
    mCropTopSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropTopSpinChanged, this);
    mCropBottomSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropBottomSliderChanged, this);
    mCropBottomSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropBottomSpinChanged, this);
    mCropLeftSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropLeftSliderChanged, this);
    mCropLeftSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropLeftSpinChanged, this);
    mCropRightSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropRightSliderChanged, this);
    mCropRightSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropRightSpinChanged, this);
    mSelectScaling->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onScalingChoiceChanged, this);
    mScalingSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onScalingSliderChanged, this);
    mScalingSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onScalingSpinChanged, this);
    mRotationSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onRotationSliderChanged, this);
    mRotationSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onRotationSpinChanged, this);
    mSelectAlignment->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onAlignmentChoiceChanged, this);
    mPositionXSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionXSliderChanged, this);
    mPositionXSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionXSpinChanged, this);
    mPositionYSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionYSliderChanged, this);
    mPositionYSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionYSpinChanged, this);

    mVolumePanel = new wxPanel(this);
    wxBoxSizer* volumesizer = new wxBoxSizer(wxHORIZONTAL);
    mBitmapVolume = new wxStaticBitmap(mVolumePanel, wxID_ANY, util::window::getBitmap("speaker-volume-blue.png"));
    mVolumeSlider = new wxSlider(mVolumePanel, wxID_ANY, model::AudioKeyFrame::sVolumeDefault, model::AudioKeyFrame::sVolumeMin, model::AudioKeyFrame::sVolumeMax );
    mVolumeSlider->SetPageSize(sVolumePageSize);
    mVolumeSlider->SetToolTip(_("Select the audio volume."));
    mBitmapVolume->SetToolTip(mVolumeSlider->GetToolTipText());
    mVolumeSpin = new wxSpinCtrl(mVolumePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mVolumeSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mVolumeSpin->SetRange(model::AudioKeyFrame::sVolumeMin, model::AudioKeyFrame::sVolumeMax);
    mVolumeSpin->SetValue(model::AudioKeyFrame::sVolumeDefault);
    mVolumeSpin->SetToolTip(mVolumeSlider->GetToolTipText());
    volumesizer->Add(mBitmapVolume, wxSizerFlags(0).Left().CenterVertical());
    volumesizer->Add(mVolumeSlider, wxSizerFlags(1).Expand());
    volumesizer->Add(mVolumeSpin, wxSizerFlags(0));
    mVolumePanel->SetSizer(volumesizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Volume (%)"), mVolumePanel, mVolumeSlider->GetToolTipText());

    mVolumeSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onVolumeSliderChanged, this);
    mVolumeSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onVolumeSpinChanged, this);

    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Audio key frames"), mAudioKeyFrameControls->mEditPanel, mAudioKeyFrameControls->mToolTip);

    mTransitionPanel = new wxPanel(this);
    mTransitionBoxSizer = new wxFlexGridSizer(getNumberOfColumns(), 0, 0);
    mTransitionBoxSizer->AddGrowableCol(2);
    mTransitionPanel->SetSizer(mTransitionBoxSizer);
    addWidget(mTransitionPanel);

    Bind(wxEVT_SHOW, &DetailsClip::onShow, this);
    Bind(wxEVT_SIZE, &DetailsClip::onSize, this);

    mBitmaps =
    {
        mBitmapLength,
        mBitmapPlayback,
        mBitmapSpeed,
        mBitmapOpacity,
        mBitmapCropTop,
        mBitmapCropBottom,
        mBitmapCropLeft,
        mBitmapCropRight,
        mBitmapRotation,
        mBitmapScaling,
        mBitmapAlignment,
        mBitmapX,
        mBitmapY,
        mBitmapVolume,
        mBitmapTransitionType,
        mVideoKeyFrameControls->mBitmapKeyFrames,
        mAudioKeyFrameControls->mBitmapKeyFrames
    };

    updateTitles();
    updateIcons();

    setClip(nullptr); // Ensures disabling all controls

    getSelection().Bind(EVENT_SELECTION_UPDATE, &DetailsClip::onSelectionChanged, this);
    getPlayer()->Bind(EVENT_PLAYBACK_POSITION, &DetailsClip::onPlaybackPosition, this);

    VAR_INFO(GetSize());
}

DetailsClip::~DetailsClip()
{
    for ( wxToggleButton* button : mLengthButtons )
    {
        button->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
    }
    getSelection().Unbind(EVENT_SELECTION_UPDATE, &DetailsClip::onSelectionChanged, this);
    getPlayer()->Unbind(EVENT_PLAYBACK_POSITION, &DetailsClip::onPlaybackPosition, this);

    mSpeedSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onSpeedSliderChanged, this);
    mSpeedSpin->Unbind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onSpeedSpinChanged, this);
    mPlayButton->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onPlayButtonPressed, this);
    mAutoPlayButton->Unbind(wxEVT_CHECKBOX, &DetailsClip::onAutoPlayToggled, this);
    mTransitionType->Unbind(wxEVT_CHOICE, &DetailsClip::onTransitionType, this);
    mOpacitySlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onOpacitySliderChanged, this);
    mOpacitySpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onOpacitySpinChanged, this);
    mCropTopSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropTopSliderChanged, this);
    mCropTopSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropTopSpinChanged, this);
    mCropBottomSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropBottomSliderChanged, this);
    mCropBottomSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropBottomSpinChanged, this);
    mCropLeftSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropLeftSliderChanged, this);
    mCropLeftSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropLeftSpinChanged, this);
    mCropRightSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onCropRightSliderChanged, this);
    mCropRightSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onCropRightSpinChanged, this);
    mSelectScaling->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onScalingChoiceChanged, this);
    mScalingSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onScalingSliderChanged, this);
    mScalingSpin->Unbind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onScalingSpinChanged, this);
    mRotationSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onRotationSliderChanged, this);
    mRotationSpin->Unbind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onRotationSpinChanged, this);
    mSelectAlignment->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onAlignmentChoiceChanged, this);
    mPositionXSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionXSliderChanged, this);
    mPositionXSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionXSpinChanged, this);
    mPositionYSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionYSliderChanged, this);
    mPositionYSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionYSpinChanged, this);
    mVolumeSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onVolumeSliderChanged, this);
    mVolumeSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onVolumeSpinChanged, this);

    Unbind(wxEVT_SHOW, &DetailsClip::onShow, this);
    Unbind(wxEVT_SIZE, &DetailsClip::onSize, this);

    mClip = nullptr;
    mTransitionClone = nullptr;
    mEditCommand = nullptr;
    mEditSpeedCommand = nullptr;

    Unbind(wxEVT_SIZE, &DetailsClip::onSize, this);
    Unbind(wxEVT_SHOW, &DetailsClip::onShow, this);
}

}} // namespace
