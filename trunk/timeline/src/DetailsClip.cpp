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

#include "DetailsClip.h"

#include "AudioClip.h"
#include "AudioClipEvent.h"
#include "ClipEvent.h"
#include "ClipPreview.h"
#include "Combiner.h"
#include "CommandProcessor.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "DetailsPanel.h"
#include "EditClipDetails.h"
#include "IClip.h"
#include "Player.h"
#include "Selection.h"
#include "SelectionEvent.h"
#include "Sequence.h"
#include "StatusBar.h"
#include "Transition.h"
#include "TransitionParameter.h"
#include "Trim.h"
#include "TrimClip.h"
#include "UtilClone.h"
#include "UtilEnumSelector.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoClipEvent.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const double sSpeedIncrement = 0.01;
const double sScalingIncrement = 0.01;
const double sRotationIncrement = 0.01;
const int sSpeedPageSize = 10;
const int sPositionPageSize = 10;
const int sOpacityPageSize = 10;
const int sVolumePageSize = 10;
const wxString sVideo(_("Video"));
const wxString sAudio(_("Audio"));
const wxString sTransition(_("Transition"));

const wxString sEditOpacity(_("Edit opacity of "));
const wxString sEditScalingType(_("Edit automated scaling of "));
const wxString sEditScaling(_("Edit scaling of "));
const wxString sEditRotation(_("Edit rotation of "));
const wxString sEditAlignment(_("Edit automated alignment of "));
const wxString sEditX(_("Edit X position of "));
const wxString sEditY(_("Edit Y position of "));
const wxString sEditVolume(_("Edit volume of "));


DetailsClip::DetailsClip(wxWindow* parent, Timeline& timeline)
    : DetailsPanel(parent, timeline)
{
    VAR_DEBUG(this);

    addBox(boost::none, 2);

    {
        // Read lengths from config
        wxString lengthButtons = wxConfigBase::Get()->Read(Config::sPathTimelineLengthButtons, "");
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
            wxConfigBase::Get()->Write(Config::sPathTimelineLengthButtons, configValue);
            wxConfigBase::Get()->Flush();
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
        button->SetToolTip(_("Change the length of the clip to this length. Will shift other clips to avoid introducing a black area.") + _(" Shortcut key: ") + "'" + wxString::Format("%d", i + 1) + "'");
        mLengthPanel->GetSizer()->Add(button,wxSizerFlags(1));
        button->Bind( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
        mLengthButtons.push_back(button);
    }
    updateLengthButtons();
    addOption(_("Fixed lengths (s)"), mLengthPanel);

    mSpeedPanel = new wxPanel(this);
    wxBoxSizer* speedsizer = new wxBoxSizer(wxHORIZONTAL);
    mSpeedSlider = new wxSlider(mSpeedPanel, wxID_ANY, model::Constants::sSpeedMax, model::Constants::sSpeedMin, model::Constants::sSpeedMax );
    mSpeedSlider->SetPageSize(sSpeedPageSize);
    mSpeedSpin = new wxSpinCtrlDouble(mSpeedPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mSpeedSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mSpeedSpin->SetDigits(model::Constants::sSpeedPrecision);
    mSpeedSpin->SetValue(1);
    mSpeedSpin->SetRange(
        static_cast<double>(model::Constants::sSpeedMin) / static_cast<double>(model::Constants::sSpeedPrecisionFactor),
        static_cast<double>(model::Constants::sSpeedMax) / static_cast<double>(model::Constants::sSpeedPrecisionFactor));
    mSpeedSpin->SetIncrement(sSpeedIncrement);
    speedsizer->Add(mSpeedSlider, wxSizerFlags(1).Expand());
    speedsizer->Add(mSpeedSpin, wxSizerFlags(0));
    mSpeedPanel->SetSizer(speedsizer);
    addOption(_("Speed"), mSpeedPanel);

    mSpeedSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onSpeedSliderChanged, this);
    mSpeedSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onSpeedSpinChanged, this);

    mOpacityPanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mOpacitySlider = new wxSlider(mOpacityPanel, wxID_ANY, model::Constants::sOpacityMax, model::Constants::sOpacityMin, model::Constants::sOpacityMax );
    mOpacitySlider->SetPageSize(sOpacityPageSize);
    mOpacitySpin = new wxSpinCtrl(mOpacityPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mOpacitySpin->SetRange(model::Constants::sOpacityMin, model::Constants::sOpacityMax);
    mOpacitySpin->SetValue(model::Constants::sOpacityMax);
    mOpacitySpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    opacitysizer->Add(mOpacitySlider, wxSizerFlags(1).Expand());
    opacitysizer->Add(mOpacitySpin, wxSizerFlags(0));
    mOpacityPanel->SetSizer(opacitysizer);
    addOption(_("Opacity"), mOpacityPanel);

    mRotationPanel = new wxPanel(this);
    wxBoxSizer* rotationsizer = new wxBoxSizer(wxHORIZONTAL);
    mRotationSlider = new wxSlider(mRotationPanel,wxID_ANY, 1 * model::Constants::sRotationPrecisionFactor, model::Constants::sRotationMin, model::Constants::sRotationMax);
    mRotationSlider->SetPageSize(model::Constants::sRotationPageSize);
    mRotationSpin = new wxSpinCtrlDouble(mRotationPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mRotationSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mRotationSpin->SetDigits(model::Constants::sRotationPrecision);
    mRotationSpin->SetValue(0); // No rotation
    mRotationSpin->SetRange(
        static_cast<double>(model::Constants::sRotationMin) / static_cast<double>(model::Constants::sRotationPrecisionFactor),
        static_cast<double>(model::Constants::sRotationMax) / static_cast<double>(model::Constants::sRotationPrecisionFactor));
    mRotationSpin->SetIncrement(sRotationIncrement);
    rotationsizer->Add(mRotationSlider, wxSizerFlags(1).Expand());
    rotationsizer->Add(mRotationSpin, wxSizerFlags(0));
    mRotationPanel->SetSizer(rotationsizer);
    addOption(_("Rotation"), mRotationPanel);

    mScalingPanel = new wxPanel(this);
    wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
    mSelectScaling = new EnumSelector<model::VideoScaling>(mScalingPanel, model::VideoScalingConverter::mapToHumanReadibleString, model::VideoScalingNone);
    mSelectScaling->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSlider = new wxSlider(mScalingPanel,wxID_ANY, 1 * model::Constants::sScalingPrecisionFactor, model::Constants::sScalingMin, model::Constants::sScalingMax);
    mScalingSlider->SetPageSize(model::Constants::sScalingPageSize);
    mScalingSpin = new wxSpinCtrlDouble(mScalingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mScalingSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSpin->SetValue(1); // No scaling
    mScalingSpin->SetDigits(model::Constants::sScalingPrecision);
    mScalingSpin->SetRange(
        static_cast<double>(model::Constants::sScalingMin) / static_cast<double>(model::Constants::sScalingPrecisionFactor),
        static_cast<double>(model::Constants::sScalingMax) / static_cast<double>(model::Constants::sScalingPrecisionFactor));
    mScalingSpin->SetIncrement(sScalingIncrement);
    scalingsizer->Add(mSelectScaling, wxSizerFlags(0).Left());
    scalingsizer->Add(mScalingSlider, wxSizerFlags(1).Expand());
    scalingsizer->Add(mScalingSpin, wxSizerFlags(0));
    mScalingPanel->SetSizer(scalingsizer);
    addOption(_("Scaling"), mScalingPanel);

    mAlignmentPanel = new wxPanel(this);
    wxBoxSizer* alignmentsizer = new wxBoxSizer(wxHORIZONTAL);
    mSelectAlignment = new EnumSelector<model::VideoAlignment>(mAlignmentPanel, model::VideoAlignmentConverter::mapToHumanReadibleString, model::VideoAlignmentCustom);
    mSelectAlignment->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    wxStaticText* titleX = new wxStaticText(mAlignmentPanel, wxID_ANY, _("  X:"), wxDefaultPosition);
    mPositionXSlider = new wxSlider(mAlignmentPanel, wxID_ANY, 0, 0, 1);
    mPositionXSlider->SetPageSize(sPositionPageSize);
    mPositionXSpin = new wxSpinCtrl(mAlignmentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionXSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionXSpin->SetRange(0,1);
    mPositionXSpin->SetValue(0);
    wxStaticText* titleY = new wxStaticText(mAlignmentPanel, wxID_ANY, _("  Y:"), wxDefaultPosition);
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
    addOption(_("Position"), mAlignmentPanel);

    // Give these two the same width
    mSelectAlignment->SetMinSize(wxSize(mSelectScaling->GetSize().x,-1));
    mSelectScaling->SetMinSize(wxSize(mSelectAlignment->GetSize().x,-1));

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
    mVolumeSlider = new wxSlider(mVolumePanel, wxID_ANY, model::Constants::sDefaultVolume, model::Constants::sMinVolume, model::Constants::sMaxVolume );
    mVolumeSlider->SetPageSize(sVolumePageSize);
    mVolumeSpin = new wxSpinCtrl(mVolumePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mVolumeSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mVolumeSpin->SetRange(model::Constants::sMinVolume, model::Constants::sMaxVolume);
    mVolumeSpin->SetValue(model::Constants::sMaxVolume);
    volumesizer->Add(mVolumeSlider, wxSizerFlags(1).Expand());
    volumesizer->Add(mVolumeSpin, wxSizerFlags(0));
    mVolumePanel->SetSizer(volumesizer);
    addOption(_("Volume (%)"), mVolumePanel);

    mVolumeSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onVolumeSliderChanged, this);
    mVolumeSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onVolumeSpinChanged, this);

    mTransitionBoxSizer = addBox(boost::none);

    Bind(wxEVT_SHOW, &DetailsClip::onShow, this);

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
    VAR_INFO(GetSize());
}

DetailsClip::~DetailsClip()
{
    for ( wxToggleButton* button : mLengthButtons )
    {
        button->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
    }
    getSelection().Unbind(EVENT_SELECTION_UPDATE, &DetailsClip::onSelectionChanged, this);

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

    mClip.reset();
    mClones.reset();
    mEditCommand = nullptr;

    Unbind(wxEVT_SHOW, &DetailsClip::onShow, this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr DetailsClip::getClip() const
{
    return mClip;
}

void DetailsClip::setClip(const model::IClipPtr& clip)
{
    VAR_DEBUG(clip);

    if (mClip == clip) return; // Avoid useless updating

    mClip.reset();
    mClones.reset();
    mEditCommand = nullptr;

    model::VideoClipPtr video =
        !clip ? nullptr :
        clip->isA<model::VideoClip>() ? boost::dynamic_pointer_cast<model::VideoClip>(clip) :
        clip->isA<model::AudioClip>() ? boost::dynamic_pointer_cast<model::VideoClip>(clip->getLink()) :
        nullptr;
    model::AudioClipPtr audio = 
        !clip ? nullptr :
        clip->isA<model::AudioClip>() ? boost::dynamic_pointer_cast<model::AudioClip>(clip) :
        clip->isA<model::VideoClip>() ? boost::dynamic_pointer_cast<model::AudioClip>(clip->getLink()) :
        nullptr;
    model::TransitionPtr transition = 
        !clip ? nullptr :
        clip->isA<model::Transition>() ? boost::dynamic_pointer_cast<model::Transition>(clip) :
        nullptr;

    if (video != nullptr ||
        audio != nullptr ||
        transition != nullptr)
    {
        // Clip type for which editing details is supported.
        mClip = clip;

        // Length/speed
        // For audio/video clips and transitions, the length can be edited.
        determineClipSizeBounds();
        updateLengthButtons();

        if (video || audio)
        {
            boost::rational< int > speed = boost::dynamic_pointer_cast<model::ClipInterval>(mClip)->getSpeed(); // todo what if transition? then poof
            boost::rational<int> maxSpeed = 10; // todo mEditCommand->getMaxVideoSpeed(); // todo only one slider for speed, only enabled for audio-video with same length
            mSpeedSlider->SetValue(boost::rational_cast<int>(speed * model::Constants::sSpeedPrecisionFactor));
            mSpeedSpin->SetValue(boost::rational_cast<double>(speed));
            mSpeedSlider->SetMax(boost::rational_cast<int>(maxSpeed * model::Constants::sSpeedPrecisionFactor));
            mSpeedSpin->SetRange(
                static_cast<double>(model::Constants::sSpeedMin) / static_cast<double>(model::Constants::sSpeedPrecisionFactor),
                boost::rational_cast<double>(maxSpeed));
        }

        if (video)
        {
            wxSize originalSize = video->getInputSize();
            boost::rational< int > factor = video->getScalingFactor();
            boost::rational< int > rotation = video->getRotation();
            wxPoint position = video->getPosition();
            wxPoint maxpos = video->getMaxPosition();
            wxPoint minpos = video->getMinPosition();
            int opacity = video->getOpacity();
            const double sScalingIncrement = 0.01;

            mOpacitySlider->SetValue(opacity);
            mOpacitySpin->SetValue(opacity);

            mSelectScaling->select(video->getScaling());
            double sliderFactor = boost::rational_cast<double>(factor);
            mScalingSlider->SetValue(boost::rational_cast<int>(factor * model::Constants::sScalingPrecisionFactor));
            mScalingSpin->SetValue(sliderFactor);

            double angle = boost::rational_cast<double>(rotation);
            mRotationSlider->SetValue(boost::rational_cast<int>(rotation * model::Constants::sRotationPrecisionFactor));
            mRotationSpin->SetValue(angle);

            mSelectAlignment->select(video->getAlignment());
            mPositionXSlider->SetRange(minpos.x, maxpos.x);
            mPositionXSlider->SetValue(position.x);
            mPositionXSpin->SetRange(minpos.x, maxpos.x);
            mPositionXSpin->SetValue(position.x);
            mPositionYSlider->SetRange(minpos.y, maxpos.y);
            mPositionYSlider->SetValue(position.y);
            mPositionYSpin->SetRange(minpos.y, maxpos.y);
            mPositionYSpin->SetValue(position.y);
        }

        if (audio)
        {
            mVolumeSlider->SetValue(audio->getVolume());
            mVolumeSpin->SetValue(audio->getVolume());
        }

        // NOT: if (mClones->Transition) -- the transition controls are added dynamically in ClonesContainer

        requestShow(true, mClip->getDescription() + " (" + model::Convert::ptsToHumanReadibleString(mClip->getPerceivedLength()) + "s)");
    }
    else
    {
        requestShow(false);
    }

    // Note: disabling a control and then enabling it again can cause extra events (value changed).
    // Therefore this has been placed here, to only dis/enable in the minimal number of cases.
    showOption(mLengthPanel, video != nullptr || audio  != nullptr || transition != nullptr);
    showOption(mSpeedPanel, video  != nullptr|| audio != nullptr);
    showOption(mOpacityPanel, video != nullptr);
    showOption(mScalingPanel, video != nullptr);
    showOption(mRotationPanel, video != nullptr);
    showOption(mAlignmentPanel, video != nullptr);
    showOption(mVolumePanel, audio != nullptr);

    Layout();
}

pts DetailsClip::getLength(wxToggleButton* button) const
{
    ASSERT_LESS_THAN(button->GetId(), static_cast<int>(mLengths.size()));
    return model::Convert::timeToPts(mLengths[button->GetId()]);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onShow(wxShowEvent& event)
{
    updateLengthButtons();
    event.Skip();
}

void DetailsClip::onLengthButtonPressed(wxCommandEvent& event)
{
    wxToggleButton* button = dynamic_cast<wxToggleButton*>(event.GetEventObject());
    handleLengthButtonPressed(button);
    event.Skip();
}

void DetailsClip::onSpeedSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mSpeedSlider->GetValue());
    //todo
    //command::EditClipDetails& command = getEditCommand(EditClipAspectClipSpeed);
    //boost::rational<int> r(mSpeedSlider->GetValue(), model::Constants::sSpeedPrecisionFactor);
    //command.getVideo()->setSpeed(r);
    event.Skip();
}

void DetailsClip::onSpeedSpinChanged(wxSpinDoubleEvent& event)
{
    double value = mSpeedSpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    int spinFactor = floor(value * model::Constants::sSpeedPrecisionFactor);
    // todo
    //command::EditClipDetails& command = getEditCommand(EditClipAspectClipSpeed); // Use same widget to only update once, regardless of the originating control
    boost::rational<int> r(spinFactor, model::Constants::sSpeedPrecisionFactor);
    //command.getVideo()->setSpeed(r);
    event.Skip();
}

void DetailsClip::onOpacitySliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mOpacitySlider->GetValue());
    VAR_ERROR(mOpacitySlider->GetValue());
    submitEditCommandUponFirstEdit(sEditOpacity);
    mClones->Video->setOpacity(mOpacitySlider->GetValue());
    event.Skip();
}

void DetailsClip::onOpacitySpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mOpacitySpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    submitEditCommandUponFirstEdit(sEditOpacity); // Changes same clip aspect as the slider
    mClones->Video->setOpacity(mOpacitySpin->GetValue());
    event.Skip();
}

void DetailsClip::onScalingChoiceChanged(wxCommandEvent& event)
{
    VAR_INFO(mSelectScaling->getValue());
    submitEditCommandUponFirstEdit(sEditScalingType);
    mClones->Video->setScaling(mSelectScaling->getValue(), boost::none);
    event.Skip();
}

void DetailsClip::onScalingSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mScalingSlider->GetValue());
    boost::rational<int> r(mScalingSlider->GetValue(), model::Constants::sScalingPrecisionFactor);
    submitEditCommandUponFirstEdit(sEditScaling);
    mClones->Video->setScaling(model::VideoScalingCustom, boost::optional< boost::rational< int > >(r));
    event.Skip();
}

void DetailsClip::onScalingSpinChanged(wxSpinDoubleEvent& event)
{
    VAR_INFO(mScalingSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    int spinFactor = floor(mScalingSpin->GetValue() * model::Constants::sScalingPrecisionFactor);
    boost::rational<int> r(spinFactor, model::Constants::sScalingPrecisionFactor);
    submitEditCommandUponFirstEdit(sEditScaling); // Changes same clip aspect as the slider
    mClones->Video->setScaling(model::VideoScalingCustom, boost::optional< boost::rational< int > >(r));
    event.Skip();
}

void DetailsClip::onRotationSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mRotationSlider->GetValue());
    boost::rational<int> r(mRotationSlider->GetValue(), model::Constants::sRotationPrecisionFactor);
    submitEditCommandUponFirstEdit(sEditRotation);
    mClones->Video->setRotation(r);
    event.Skip();
}

void DetailsClip::onRotationSpinChanged(wxSpinDoubleEvent& event)
{
    VAR_INFO(mRotationSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    int spinFactor = floor(mRotationSpin->GetValue() * model::Constants::sRotationPrecisionFactor);
    boost::rational<int> r(spinFactor, model::Constants::sRotationPrecisionFactor);
    submitEditCommandUponFirstEdit(sEditRotation); // Changes same clip aspect as the slider
    mClones->Video->setRotation(r);
    event.Skip();
}

void DetailsClip::onAlignmentChoiceChanged(wxCommandEvent& event)
{
    VAR_INFO(mSelectAlignment->getValue());
    submitEditCommandUponFirstEdit(sEditAlignment);
    mClones->Video->setAlignment(mSelectAlignment->getValue());
    event.Skip();
}

void DetailsClip::onPositionXSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionXSlider->GetValue());
    submitEditCommandUponFirstEdit(sEditX);
    updateAlignment(true);
    mClones->Video->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionXSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mPositionXSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    submitEditCommandUponFirstEdit(sEditX); // Changes same clip aspect as the slider
    updateAlignment(true);
    mClones->Video->setPosition(wxPoint(mPositionXSpin->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionYSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionYSlider->GetValue());
    submitEditCommandUponFirstEdit(sEditY);
    updateAlignment(false);
    mClones->Video->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionYSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mPositionYSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    submitEditCommandUponFirstEdit(sEditY); // Changes same clip aspect as the slider
    updateAlignment(false);
    mClones->Video->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSpin->GetValue()));
    event.Skip();
}

void DetailsClip::onVolumeSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mVolumeSlider->GetValue());
    submitEditCommandUponFirstEdit(sEditVolume);
    mClones->Audio->setVolume(mVolumeSlider->GetValue());
    event.Skip();
}

void DetailsClip::onVolumeSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mVolumeSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    submitEditCommandUponFirstEdit(sEditVolume); // Changes same clip aspect as the slider
    mClones->Audio->setVolume(mVolumeSpin->GetValue());
    event.Skip();
}

void DetailsClip::onTimelineKey(int keycode)
{
    switch (keycode)
    {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            unsigned int index = keycode - '1';
            if (index < mLengthButtons.size())
            {
                VAR_DEBUG(index);
                handleLengthButtonPressed(mLengthButtons[index]);
            }
            break;
        }
    }
}

void DetailsClip::handleLengthButtonPressed(wxToggleButton* button)
{
    ASSERT_NONZERO(button);
    ASSERT(wxThread::IsMain());
    if (!button->IsEnabled()) { return; }
    pts length = getLength(button);
    VAR_INFO(length);
    ASSERT(mTrimAtEnd.find(length) != mTrimAtEnd.end())(mTrimAtEnd)(length);
    ASSERT(mTrimAtBegin.find(length) != mTrimAtBegin.end())(mTrimAtBegin)(length);

    ::command::Combiner* command = new ::command::Combiner();

    getTimeline().beginTransaction();

    model::IClipPtr clip = mClip;
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
    pts endtrim = mTrimAtEnd[length];
    pts begintrim = mTrimAtBegin[length];
    bool shift = !transition;
    bool error = false;

    if (endtrim != 0)
    {
        ::gui::timeline::command::TrimClip* trimCommand = new command::TrimClip(getSequence(), clip, model::TransitionPtr(), transition ? TransitionEnd : ClipEnd);
        trimCommand->update(endtrim, shift, true);
        clip = trimCommand->getNewClip();
        transition = boost::dynamic_pointer_cast<model::Transition>(clip);
        command->add(trimCommand);
    }

    if (begintrim != 0)
    {
        if (endtrim != 0)
        {
            // When determining the trim boundaries, end trim and begin trim boundaries are determined without taking into account that both sides may be trimmed.
            // That may cause certain trim operations to cause unwanted results. Instead of applying one trim and then determining the final trim result, or trying
            // another 'trick', before doing the second trim here, an additional boundaries check is done here.
            //
            // The differences between the original computation and the result here are particularly related to shifting clips in other tracks and
            // related to having multiple transitions besides the clip.
            command::TrimClip::TrimLimit limitsBeginTrim;
            if (transition)
            {
                limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionBegin, false);
            }
            else
            {
                limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), clip, clip->getLink(), ClipBegin, true);
            }
            error = (begintrim < limitsBeginTrim.Min || begintrim > limitsBeginTrim.Max);
        }
        if (!error)
        {
            ::gui::timeline::command::TrimClip* trimCommand = new command::TrimClip(getSequence(), clip, model::TransitionPtr(), transition ? TransitionBegin : ClipBegin);
            trimCommand->update(begintrim, shift, true);
            clip = trimCommand->getNewClip();
            command->add(trimCommand);
        }
    }

    if (error)
    {
        gui::StatusBar::get().timedInfoText(_("Could not execute the length change without messing up clips in other tracks."));
        delete command; // Do not execute. Undo any changes.
        clip = mClip;
    }
    else
    {
        command->setName(_("Set length of clip"));
        command->submit();
    }

    // It might be possible that a new length selection button has already been pressed
    // and it's button event is already queued. When that event is handled this new clip
    // must be used.
    setClip(clip);
    //NOT: updateLengthButtons(); -- this is automatically done after selecting a new clip
    getTimeline().endTransaction();
    getTimeline().Refresh();
    getTimeline().SetFocus();

}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onSpeedChanged(model::EventChangeClipSpeed& event)
{
    mSpeedSpin->SetValue(boost::rational_cast<double>(event.getValue()));
    mSpeedSlider->SetValue(floor(event.getValue() * model::Constants::sSpeedPrecisionFactor));
    preview();
    event.Skip();
}

void DetailsClip::onOpacityChanged(model::EventChangeVideoClipOpacity& event)
{
    mOpacitySlider->SetValue(event.getValue());
    mOpacitySpin->SetValue(event.getValue());
    preview();
    event.Skip();
}

void DetailsClip::onScalingChanged(model::EventChangeVideoClipScaling& event)
{
    mSelectScaling->select(event.getValue());
    event.Skip();
}

void DetailsClip::onScalingFactorChanged(model::EventChangeVideoClipScalingFactor& event)
{
    mScalingSpin->SetValue(boost::rational_cast<double>(event.getValue()));
    mScalingSlider->SetValue(floor(event.getValue() * model::Constants::sScalingPrecisionFactor));
    preview();
    event.Skip();
}

void DetailsClip::onRotationChanged(model::EventChangeVideoClipRotation& event)
{
    mRotationSpin->SetValue(boost::rational_cast<double>(event.getValue()));
    mRotationSlider->SetValue(floor(event.getValue() * model::Constants::sRotationPrecisionFactor));
    preview();
    event.Skip();
}

void DetailsClip::onAlignmentChanged(model::EventChangeVideoClipAlignment& event)
{
    mSelectAlignment->select(event.getValue());
    preview();
    event.Skip();
}

void DetailsClip::onPositionChanged(model::EventChangeVideoClipPosition& event)
{
    mPositionXSpin->SetValue(event.getValue().x);
    mPositionXSlider->SetValue(event.getValue().x);
    mPositionYSpin->SetValue(event.getValue().y);
    mPositionYSlider->SetValue(event.getValue().y);
    preview();
    event.Skip();
}

void DetailsClip::onMinPositionChanged(model::EventChangeVideoClipMinPosition& event)
{
    mPositionXSpin->SetRange(event.getValue().x,mPositionXSpin->GetMax());
    mPositionYSpin->SetRange(event.getValue().y,mPositionYSpin->GetMax());
    mPositionXSlider->SetRange(event.getValue().x,mPositionXSlider->GetMax());
    mPositionYSlider->SetRange(event.getValue().y,mPositionYSlider->GetMax());
    event.Skip();
}

void DetailsClip::onMaxPositionChanged(model::EventChangeVideoClipMaxPosition& event)
{
    mPositionXSpin->SetRange(mPositionXSpin->GetMin(),event.getValue().x);
    mPositionYSpin->SetRange(mPositionYSpin->GetMin(), event.getValue().y);
    mPositionXSlider->SetRange(mPositionXSlider->GetMin(),event.getValue().x);
    mPositionYSlider->SetRange(mPositionYSlider->GetMin(), event.getValue().y);
    event.Skip();
}

void DetailsClip::onVolumeChanged(model::EventChangeAudioClipVolume& event)
{
    mVolumeSlider->SetValue(event.getValue());
    mVolumeSpin->SetValue(event.getValue());
    event.Skip();
}

void DetailsClip::onTransitionParameterChanged(model::EventTransitionParameterChanged& event)
{
    preview();//todotest
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// SELECTION EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onSelectionChanged(timeline::EventSelectionUpdate& event)
{
    VAR_DEBUG(this);
    std::set<model::IClipPtr> selection = getSequence()->getSelectedClips();
    model::IClipPtr selectedclip;
    VAR_DEBUG(selection.size());
    if (selection.size() == 1)
    {
        selectedclip = *selection.begin();
    }
    else if (selection.size() == 2)
    {
        model::IClipPtr a = *selection.begin();
        model::IClipPtr b = *(++(selection.begin()));
        if (a->getLink() == b)
        {
            ASSERT_EQUALS(b->getLink(),a);
            selectedclip = (a->isA<model::VideoClip>()) ? a : b; // Always use the videoclip (avoid problems in automated test as a result of timing differences - sometimes the videoclip is focused first, sometimes the audio clip)
        }
    }
    setClip(selectedclip);
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

std::vector<wxToggleButton*> DetailsClip::getLengthButtons() const
{
    return mLengthButtons;
}

wxSlider* DetailsClip::getOpacitySlider() const
{
    return mOpacitySlider;
}

wxSpinCtrl* DetailsClip::getOpacitySpin() const
{
    return mOpacitySpin;
}

EnumSelector<model::VideoScaling>* DetailsClip::getScalingSelector() const
{
    return mSelectScaling;
}

wxSlider* DetailsClip::getScalingSlider() const
{
    return mScalingSlider;
}

wxSpinCtrlDouble* DetailsClip::getScalingSpin() const
{
    return mScalingSpin;
}

wxSlider* DetailsClip::getRotationSlider() const
{
    return mRotationSlider;
}

wxSpinCtrlDouble* DetailsClip::getRotationSpin() const
{
    return mRotationSpin;
}

EnumSelector<model::VideoAlignment>* DetailsClip::getAlignmentSelector() const
{
    return mSelectAlignment;
}

wxSlider* DetailsClip::getPositionXSlider() const
{
    return mPositionXSlider;
}

wxSpinCtrl* DetailsClip::getPositionXSpin() const
{
    return mPositionXSpin;
}

wxSlider* DetailsClip::getPositionYSlider() const
{
    return mPositionYSlider;
}

wxSpinCtrl* DetailsClip::getPositionYSpin() const
{
    return mPositionYSpin;
}

wxSlider* DetailsClip::getVolumeSlider() const
{
    return mVolumeSlider;
}

wxSpinCtrl* DetailsClip::getVolumeSpin() const
{
    return mVolumeSpin;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::submitEditCommandUponFirstEdit(const wxString& message)
{
    getPlayer()->stop(); // Stop iteration through the sequence, since the sequence is going to be changed.

    // todo replace the large expression (make the command processor a single access class) and reuse that
    if (!mEditCommand || // No command submit yet
        mEditCommand != gui::Window::get().GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->GetCurrentCommand() || // If another command was done inbetween
        mEditCommand->getMessage() != message) // Another aspect was changed
    {
        // Use new clones for the new command
        mClones = std::make_unique<ClonesContainer>(this, mClip);

        // Create the command - which replaces the original clip(s) with their changed clones - and add it to the undo system.
        mEditCommand = new command::EditClipDetails(getSequence(), message, mClip, mClip->getLink(), mClones->Clip, mClones->Link);

        // The submission of the command will result in a newly selected clip: the clone
        // Storing that clone as the current clip serves two purposes:
        // - The first 'selectClip' after the submission is ignored (since it's the clone).
        // - Undo will actually cause mClip to be selected again.
        mClip = mClones->Clip; 

        mEditCommand->submit();

        // Do not reset mEditCommand: required for checking subsequent edits.
        // If a clip aspect is edited twice, simply adjust the clone twice,
        // but the command may only be submitted once.

        // Do not try invalidating the rectangle: mClones->Video addition event has not been received
        // by the track view class. So, no clip view/clip preview is known yet.
    }
    else
    {
        // Update the thumbnail for video (otherwise is not updated, since only one edit command is done).
        // Note that this code is executed BEFORE resetting mClones below. The reason for this is that when the new clones
        // are created, their views are not immediately initialized. However, the old clones have the same rect.
        if (mClones &&
            mClones->Video &&
            mClones->Video->getTrack())
        {
            ClipPreview* preview{ getViewMap().getClipPreview(mClip) }; // todo what if audio clip is selected???
            preview->invalidateCachedBitmaps();
            preview->invalidateRect();
            getTimeline().repaint(preview->getRect());
        }
    }

    preview();
}

// todo make it possible to add a solid color clip to the project view and timeline
// todo testrender: sync is off?

void DetailsClip::preview()
{
    if (!mClones || !mClones->Video) { return; }
 //   ASSERT_NONZERO(mClones->Video->getTrack()); // The edit command must have been submitted

    pts position = getCursor().getLogicalPosition(); // By default, show the frame under the cursor (which is already currently shown, typically)
    if ((position < mClones->Video->getLeftPts()) || 
        (position >= mClones->Video->getRightPts()))
    {
        // The cursor is not positioned under the clip being adjusted. Move the cursor to the middle frame of that clip
        position = mClones->Video->getLeftPts() + mClones->Video->getLength() / 2; // Show the middle frame of the clip
        VAR_DEBUG(position);
        getCursor().setLogicalPosition(position); // ...and move the cursor to that position
    }

    wxSize s = getPlayer()->getVideoSize();
    if (mClones->Video->getLength() > 0 &&
        s.GetWidth() > 0 &&
        s.GetHeight() > 0)
    {
        boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
        wxMemoryDC dc(*bmp);

        // Fill with black
        dc.SetBrush(wxBrush{ wxColour{ 0, 0, 0 } });
        dc.SetPen(wxPen{ wxColour{ 0, 0, 0 } });
        dc.DrawRectangle(wxPoint(0,0),dc.GetSize());

        // Draw preview of operation
        getSequence()->moveTo(position);
        ASSERT_EQUALS(dc.GetSize(),s);
        model::VideoCompositionPtr composition = getSequence()->getVideoComposition(model::VideoCompositionParameters().setBoundingBox(dc.GetSize()));
        model::VideoFramePtr compositeFrame = composition->generate();
        if (compositeFrame)
        {
            wxBitmapPtr bitmap = compositeFrame->getBitmap();
            if (bitmap)
            {
                // Don't use DrawBitmap since this gives wrong output when using wxGTK.
                wxMemoryDC dcBmp(*bitmap);
                dc.Blit(wxPoint(0,0), bitmap->GetSize(), &dcBmp, wxPoint(0,0));
            }
        }

        dc.SelectObject(wxNullBitmap);
        getPlayer()->showPreview(bmp);
    }
}

void DetailsClip::updateAlignment(bool horizontalchange)
{
    auto getAlignment = [this, horizontalchange]() -> model::VideoAlignment
    {
        switch (mSelectAlignment->getValue())
        {
        case model::VideoAlignmentCenter:           return (horizontalchange ? model::VideoAlignmentCenterVertical      : model::VideoAlignmentCenterHorizontal);
        case model::VideoAlignmentCenterHorizontal: return (horizontalchange ? model::VideoAlignmentCustom              : model::VideoAlignmentCenterHorizontal);
        case model::VideoAlignmentCenterVertical:   return (horizontalchange ? model::VideoAlignmentCenterVertical      : model::VideoAlignmentCustom);
        }
        return mSelectAlignment->getValue();
    };
    mClones->Video->setAlignment(getAlignment());
}

void DetailsClip::determineClipSizeBounds()
{
    ASSERT(mClip);

    model::IClipPtr link = mClip->getLink();
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mClip);

    command::TrimClip::TrimLimit limitsBeginTrim;
    command::TrimClip::TrimLimit limitsEndTrim;
    if (transition)
    {
        limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionBegin, false);
        limitsEndTrim = command::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionEnd, false);
    }
    else
    {
        limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), mClip, link, ClipBegin, true);
        limitsEndTrim = command::TrimClip::determineBoundaries(getSequence(), mClip, link, ClipEnd, true);
    }

    pts clipPerceivedLength = mClip->getPerceivedLength();

    mMinimumLengthWhenBeginTrimming = clipPerceivedLength + -1 * limitsBeginTrim.Max;
    mMaximumLengthWhenBeginTrimming = clipPerceivedLength  + -1 * limitsBeginTrim.Min;
    mMinimumLengthWhenEndTrimming   = clipPerceivedLength  + limitsEndTrim.Min;
    mMaximumLengthWhenEndTrimming   = clipPerceivedLength  + limitsEndTrim.Max;
    mMinimumLengthWhenBothTrimming  = clipPerceivedLength  + -1 * limitsBeginTrim.Max + limitsEndTrim.Min;
    mMaximumLengthWhenBothTrimming  = clipPerceivedLength  + -1 * limitsBeginTrim.Min + limitsEndTrim.Max;

    // The 'both trimming' values are not 100% correct (the determined boundaries don't take
    // 'trimming on both sides simultaneously' into acount, only separate single trimming.
    // This can cause the smallest buttons to be enabled sometimes, although trimming to that
    // size is not possible (particularly applies for clips that have transitions on both edges).
    // To fix these cases, the minimum required clip size is taken as a lower bound also.
    //
    // the '-' here results in the 'area required for adjacent transitions'.
    mMinimumLengthWhenBothTrimming = std::max(mMinimumLengthWhenBothTrimming, mClip->getPerceivedLength() - mClip->getLength());
    if (link)
    {
        pts linkPerceivedLength = link->getPerceivedLength();
        mMinimumLengthWhenBothTrimming = std::max(mMinimumLengthWhenBothTrimming, linkPerceivedLength - link->getLength());
        if (linkPerceivedLength != clipPerceivedLength)
        {
            // In case the two linked clips have different lengths, disable all buttons,
            // except optionallly the button for the current length.
            mMinimumLengthWhenBeginTrimming = clipPerceivedLength;
            mMaximumLengthWhenBeginTrimming = clipPerceivedLength;
            mMinimumLengthWhenEndTrimming   = clipPerceivedLength;
            mMaximumLengthWhenEndTrimming   = clipPerceivedLength;
            mMinimumLengthWhenBothTrimming  = clipPerceivedLength;
            mMaximumLengthWhenBothTrimming  = clipPerceivedLength;
        }
    }

    ASSERT_MORE_THAN_EQUALS(mMaximumLengthWhenBothTrimming, mMaximumLengthWhenEndTrimming);
    ASSERT_MORE_THAN_EQUALS(mMaximumLengthWhenBothTrimming, mMaximumLengthWhenBeginTrimming);
    ASSERT_LESS_THAN_EQUALS(mMinimumLengthWhenBothTrimming, mMinimumLengthWhenEndTrimming);
    ASSERT_LESS_THAN_EQUALS(mMinimumLengthWhenBothTrimming, mMinimumLengthWhenBeginTrimming);
    VAR_DEBUG(mMinimumLengthWhenBeginTrimming)(mMaximumLengthWhenBeginTrimming)(mMinimumLengthWhenEndTrimming)(mMaximumLengthWhenEndTrimming)(mMinimumLengthWhenBothTrimming)(mMaximumLengthWhenBothTrimming);

    // For each possible length, store if it should be achieved by trimming at the beginning or at the end (the default)
    mTrimAtEnd.clear();
    for ( wxToggleButton* button : mLengthButtons )
    {
        pts length = getLength(button);
        mTrimAtEnd[length] = 0; // Default: no trim
        mTrimAtBegin[length] = 0; // Default: no trim
        if (length != clipPerceivedLength)
        {
            if (length >= mMinimumLengthWhenEndTrimming && length <= mMaximumLengthWhenEndTrimming)
            {
                // Trim at end only - default
                mTrimAtEnd[length] = length - clipPerceivedLength;
            }
            else if (length >= mMinimumLengthWhenBeginTrimming && length <= mMaximumLengthWhenBeginTrimming)
            {
                // Trim at begin only
                mTrimAtBegin[length] = clipPerceivedLength - length;
            }
            else if (length >= mMinimumLengthWhenBothTrimming && length <= mMaximumLengthWhenBothTrimming)
            {
                if (length < clipPerceivedLength)
                {
                    // Size reduction
                    mTrimAtEnd[length] = limitsEndTrim.Min;
                    mTrimAtBegin[length] = (clipPerceivedLength - length) + limitsEndTrim.Min; // Reduce with the size used for the end trim. Note: limitsEndTrim.Min <= 0! (hence, the '+' before limitsEndTrim)
                }
                else
                {
                    // Size enlargement
                    mTrimAtEnd[length] = limitsEndTrim.Max;
                    mTrimAtBegin[length] = (clipPerceivedLength - length) + limitsEndTrim.Max; // Reduce with the size used for the end trim. Note: trim at begin <= 0! (hence, the '+' before limitsEndTrim)
                }
            }
        }
    }
}

void DetailsClip::updateLengthButtons()
{
    if (!mClip)
    {
        return;
    }
    ASSERT(!mClip->isA<model::EmptyClip>());
    for ( wxToggleButton* button : mLengthButtons )
    {
        button->SetValue(false);
        button->Disable();
    }

    pts minimumClipLength = mMinimumLengthWhenBothTrimming;
    pts maximumClipLength = mMaximumLengthWhenBothTrimming;
    pts currentLength = mClip->getPerceivedLength();
    ASSERT_MORE_THAN_EQUALS(currentLength, minimumClipLength);
    ASSERT_LESS_THAN_EQUALS(currentLength, maximumClipLength);

    for ( wxToggleButton* button : mLengthButtons )
    {
        pts length = getLength(button);
        button->SetValue(mClip && currentLength == length);
        button->Disable();

        ASSERT_MAP_CONTAINS(mTrimAtBegin,length);
        ASSERT_MAP_CONTAINS(mTrimAtEnd,length);

        if (mTrimAtEnd[length] != 0 || mTrimAtBegin[length] != 0)
        {
            button->Enable();
        }
    }
}

}} // namespace
