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
    , mVideoKeyFrameControls(std::make_shared<KeyFrameControlsImpl<model::VideoClip, model::VideoKeyFrame>>(this))
    , mAudioKeyFrameControls(std::make_shared<KeyFrameControlsImpl<model::AudioClip, model::AudioKeyFrame>>(this))
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
            mLengths.push_back(length);
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
        mLengthButtons.push_back(button);
    }
    updateLengthButtons();
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Fixed lengths (s)"), mLengthPanel);

    mSpeedPanel = new wxPanel(this);
    wxBoxSizer* speedsizer = new wxBoxSizer(wxHORIZONTAL);
    mSpeedSlider = new wxSlider(mSpeedPanel, wxID_ANY, factorToSliderValue(sFactorMax), factorToSliderValue(sFactorMin), factorToSliderValue(sFactorMax));
    mSpeedSlider->SetPageSize(sFactorPageSize);
    mSpeedSpin = new wxSpinCtrlDouble(mSpeedPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mSpeedSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mSpeedSpin->SetDigits(sFactorPrecision);
    mSpeedSpin->SetValue(1);
    mSpeedSpin->SetRange(boost::rational_cast<double>(sFactorMin), boost::rational_cast<double>(sFactorMax));
    mSpeedSpin->SetIncrement(sFactorIncrement);
    speedsizer->Add(mSpeedSlider, wxSizerFlags(1).Expand());
    speedsizer->Add(mSpeedSpin, wxSizerFlags(0));
    mSpeedPanel->SetSizer(speedsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Speed"), mSpeedPanel);

    mSpeedSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onSpeedSliderChanged, this);
    mSpeedSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onSpeedSpinChanged, this);

    mOpacityPanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mOpacitySlider = new wxSlider(mOpacityPanel, wxID_ANY, model::VideoKeyFrame::sOpacityMax, model::VideoKeyFrame::sOpacityMin, model::VideoKeyFrame::sOpacityMax );
    mOpacitySlider->SetPageSize(sOpacityPageSize);
    mOpacitySpin = new wxSpinCtrl(mOpacityPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mOpacitySpin->SetRange(model::VideoKeyFrame::sOpacityMin, model::VideoKeyFrame::sOpacityMax);
    mOpacitySpin->SetValue(model::VideoKeyFrame::sOpacityMax);
    mOpacitySpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    opacitysizer->Add(mOpacitySlider, wxSizerFlags(1).Expand());
    opacitysizer->Add(mOpacitySpin, wxSizerFlags(0));
    mOpacityPanel->SetSizer(opacitysizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Opacity"), mOpacityPanel);

    mRotationPanel = new wxPanel(this);
    wxBoxSizer* rotationsizer = new wxBoxSizer(wxHORIZONTAL);
    mRotationSlider = new wxSlider(mRotationPanel,wxID_ANY, 1 * sRotationPrecisionFactor, sRotationMinNoKeyFrames, sRotationMaxNoKeyFrames);
    mRotationSlider->SetPageSize(sRotationPageSize);
    mRotationSpin = new wxSpinCtrlDouble(mRotationPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mRotationSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mRotationSpin->SetDigits(sRotationPrecision);
    mRotationSpin->SetValue(0); // No rotation
    mRotationSpin->SetRange(
        static_cast<double>(sRotationMinNoKeyFrames) / static_cast<double>(sRotationPrecisionFactor),
        static_cast<double>(sRotationMaxNoKeyFrames) / static_cast<double>(sRotationPrecisionFactor));
    mRotationSpin->SetIncrement(sRotationIncrement);
    rotationsizer->Add(mRotationSlider, wxSizerFlags(1).Expand());
    rotationsizer->Add(mRotationSpin, wxSizerFlags(0));
    mRotationPanel->SetSizer(rotationsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Rotation"), mRotationPanel);

    mScalingPanel = new wxPanel(this);
    wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
    mSelectScaling = new EnumSelector<model::VideoScaling>(mScalingPanel, model::VideoScalingConverter::getMapToHumanReadibleString(), model::VideoScalingNone);
    mSelectScaling->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSlider = new wxSlider(mScalingPanel,wxID_ANY, factorToSliderValue(model::VideoKeyFrame::sScalingMax), factorToSliderValue(model::VideoKeyFrame::sScalingMin), factorToSliderValue(model::VideoKeyFrame::sScalingMax));
    mScalingSlider->SetPageSize(sFactorPageSize);
    mScalingSpin = new wxSpinCtrlDouble(mScalingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mScalingSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSpin->SetValue(1); // No scaling
    mScalingSpin->SetDigits(sFactorPrecision);
    mScalingSpin->SetRange(boost::rational_cast<double>(model::VideoKeyFrame::sScalingMin), boost::rational_cast<double>(model::VideoKeyFrame::sScalingMax));
    mScalingSpin->SetIncrement(sFactorIncrement);
    scalingsizer->Add(mSelectScaling, wxSizerFlags(0).Left());
    scalingsizer->Add(mScalingSlider, wxSizerFlags(1).Expand());
    scalingsizer->Add(mScalingSpin, wxSizerFlags(0));
    mScalingPanel->SetSizer(scalingsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Scaling"), mScalingPanel);

    mAlignmentPanel = new wxPanel(this);
    wxBoxSizer* alignmentsizer = new wxBoxSizer(wxHORIZONTAL);
    mSelectAlignment = new EnumSelector<model::VideoAlignment>(mAlignmentPanel, model::VideoAlignmentConverter::getMapToHumanReadibleString(), model::VideoAlignmentCustom);
    mSelectAlignment->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    wxStaticText* titleX = new wxStaticText(mAlignmentPanel, wxID_ANY, "  X:", wxDefaultPosition);
    mPositionXSlider = new wxSlider(mAlignmentPanel, wxID_ANY, 0, 0, 1);
    mPositionXSlider->SetPageSize(sPositionPageSize);
    mPositionXSpin = new wxSpinCtrl(mAlignmentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionXSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionXSpin->SetRange(0,1);
    mPositionXSpin->SetValue(0);
    wxStaticText* titleY = new wxStaticText(mAlignmentPanel, wxID_ANY, "  Y:", wxDefaultPosition);
    mPositionYSlider = new wxSlider(mAlignmentPanel, wxID_ANY, 0, 0, 1);
    mPositionYSlider->SetPageSize(sPositionPageSize);
    mPositionYSpin = new wxSpinCtrl(mAlignmentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionYSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionYSpin->SetRange(0,1);
    mPositionYSpin->SetValue(0);
    alignmentsizer->Add(mSelectAlignment, wxSizerFlags(0).Expand());
    alignmentsizer->Add(titleX, wxSizerFlags(0).Expand());
    alignmentsizer->Add(mPositionXSlider, wxSizerFlags(1000).Expand());
    alignmentsizer->Add(mPositionXSpin, wxSizerFlags(0).Expand());
    alignmentsizer->Add(titleY, wxSizerFlags(0).Expand());
    alignmentsizer->Add(mPositionYSlider, wxSizerFlags(1000));
    alignmentsizer->Add(mPositionYSpin, wxSizerFlags(0).Expand());
    mAlignmentPanel->SetSizer(alignmentsizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Position"), mAlignmentPanel);

    // Give these two the same width
    mSelectAlignment->SetMinSize(wxSize(mSelectScaling->GetSize().x,-1));
    mSelectScaling->SetMinSize(wxSize(mSelectAlignment->GetSize().x,-1));

    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Video key frames"), mVideoKeyFrameControls->mEditPanel);

    mOpacitySlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onOpacitySliderChanged, this);
    mOpacitySpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onOpacitySpinChanged, this);
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
    mVolumeSlider = new wxSlider(mVolumePanel, wxID_ANY, model::AudioKeyFrame::sVolumeDefault, model::AudioKeyFrame::sVolumeMin, model::AudioKeyFrame::sVolumeMax );
    mVolumeSlider->SetPageSize(sVolumePageSize);
    mVolumeSpin = new wxSpinCtrl(mVolumePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mVolumeSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mVolumeSpin->SetRange(model::AudioKeyFrame::sVolumeMin, model::AudioKeyFrame::sVolumeMax);
    mVolumeSpin->SetValue(model::AudioKeyFrame::sVolumeDefault);
    volumesizer->Add(mVolumeSlider, wxSizerFlags(1).Expand());
    volumesizer->Add(mVolumeSpin, wxSizerFlags(0));
    mVolumePanel->SetSizer(volumesizer);
    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Volume (%)"), mVolumePanel);

    mVolumeSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onVolumeSliderChanged, this);
    mVolumeSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onVolumeSpinChanged, this);

    // TRANSLATORS: Do not let the string exceed 20 characters.
    addOption(_("Audio key frames"), mAudioKeyFrameControls->mEditPanel);

    mTransitionBoxSizer = addBox(boost::none);

    Bind(wxEVT_SHOW, &DetailsClip::onShow, this);
    Bind(wxEVT_SIZE, &DetailsClip::onSize, this);

    // This very small button was an experiment to make 'hidable' panels for duration/video/etc.
    //addbox(_("Audio"));
    //wxPanel* p = new wxPanel();
    //wxBoxSizer* b = new wxBoxSizer(wxVERTICAL);
    //wxButton* ff = new wxButton(this,wxID_ANY,_("label"),wxDefaultPosition,wxDefaultSize, wxBORDER_RAISED| wxBU_EXACTFIT/* | wxBU_TOP*/);
    //wxFont font = ff->GetFont();
    //int i = font.GetPointSize();
    //font.Scale(0.9f);
    //font.Bold();
    //ff->SetFont(font);
    //ff->SetSize(-1,16);
    //b->Add(ff,wxSizerFlags(0).Top());
    //p->SetSizerAndFit(b);
    //addoption(_("test"), p);

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
    mOpacitySlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onOpacitySliderChanged, this);
    mOpacitySpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onOpacitySpinChanged, this);
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

    mClip = nullptr;
    mTransitionClone = nullptr;
    mEditCommand = nullptr;
    mEditSpeedCommand = nullptr;

    Unbind(wxEVT_SIZE, &DetailsClip::onSize, this);
    Unbind(wxEVT_SHOW, &DetailsClip::onShow, this);
}

}} // namespace
