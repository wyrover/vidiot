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
#include "ChangeAudioClipVolume.h"
#include "ChangeVideoClipTransform.h"
#include "Combiner.h"
#include "CommandProcessor.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "DetailsPanel.h"
#include "IClip.h"
#include "Layout.h"
#include "Player.h"
#include "Selection.h"
#include "SelectionEvent.h"
#include "Sequence.h"
#include "StatusBar.h"
#include "Transition.h"
#include "Trim.h"
#include "TrimClip.h"
#include "UtilClone.h"
#include "UtilEnumSelector.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoClipEvent.h"
#include "VideoFrame.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"

namespace gui { namespace timeline {
template <class TARGET>
boost::shared_ptr<TARGET> getTypedClip(model::IClipPtr clip)
{
    if (clip->isA<TARGET>())
    {
        return boost::dynamic_pointer_cast<TARGET>(clip);
    }
    else
    {
        return boost::dynamic_pointer_cast<TARGET>(clip->getLink());
    }
    return boost::shared_ptr<TARGET>();
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const double sScalingIncrement = 0.01;
const double sRotationIncrement = 0.01;
const int sPositionPageSize = 10;
const int sOpacityPageSize = 10;
const int sVolumePageSize = 10;
const wxString sVideo(_("Video"));
const wxString sAudio(_("Audio"));
const wxString sTransition(_("Transition"));

DetailsClip::DetailsClip(wxWindow* parent, Timeline& timeline)
    :   DetailsPanel(parent,timeline)
    ,   mClip()
    ,   mVideoClip()
    ,   mAudioClip()
    ,   mTransition()
    ,   mCurrentLength()
    ,   mOpacitySlider(0)
    ,   mOpacitySpin(0)
    ,   mSelectScaling(0)
    ,   mScalingSlider(0)
    ,   mScalingSpin(0)
    ,   mRotationSlider(0)
    ,   mRotationSpin(0)
    ,   mSelectAlignment(0)
    ,   mPositionXSpin(0)
    ,   mPositionXSlider(0)
    ,   mPositionYSpin(0)
    ,   mPositionYSlider(0)
    ,   mTransformCommand(0)
    ,   mVolumeCommand(0)
    ,   mMinimumLengthWhenBeginTrimming(0)
    ,   mMaximumLengthWhenBeginTrimming(0)
    ,   mMinimumLengthWhenEndTrimming(0)
    ,   mMaximumLengthWhenEndTrimming(0)
    ,   mMinimumLengthWhenBothTrimming(0)
    ,   mMaximumLengthWhenBothTrimming(0)
    ,   mVolumeSlider(0)
    ,   mVolumeSpin(0)
{
    VAR_DEBUG(this);

    addBox(_("Duration"));

    mCurrentLength = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxST_NO_AUTORESIZE);
    addOption(_("Current length"), mCurrentLength);

    std::vector<int> defaultLengths = { 250, 500, 1000, 1500, 2000, 2500, 3000 };
    wxStrings labels = { "0.25", "0.5", "1.0", "1.5", "2.0", "2.5", "3.0" };
    wxPanel* lengthbuttonspanel = new wxPanel(this);
    lengthbuttonspanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    auto it = defaultLengths.begin();
    auto itLabel = labels.begin();
    for (auto it = defaultLengths.begin(); it != defaultLengths.end(); ++it, ++itLabel)
    {
        int length = *it;
        // Use the integer as id
        wxToggleButton* button = new wxToggleButton(lengthbuttonspanel, length, *itLabel, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        button->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
        button->SetToolTip(_("Change the length of the clip to this length. Will shift other clips to avoid introducing a black area."));
        lengthbuttonspanel->GetSizer()->Add(button,wxSizerFlags(1));
        button->Bind( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
        mLengthButtons.push_back(button);
    }
    updateLengthButtons();
    addOption(_("Fixed lengths (s)"),lengthbuttonspanel);

    addBox(sVideo);

    wxPanel* opacitypanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mOpacitySlider = new wxSlider(opacitypanel, wxID_ANY, model::Constants::sOpacityMax, model::Constants::sOpacityMin, model::Constants::sOpacityMax );
    mOpacitySlider->SetPageSize(sOpacityPageSize);
    mOpacitySpin = new wxSpinCtrl(opacitypanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mOpacitySpin->SetRange(model::Constants::sOpacityMin, model::Constants::sOpacityMax);
    mOpacitySpin->SetValue(model::Constants::sOpacityMax);
    mOpacitySpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    opacitysizer->Add(mOpacitySlider, wxSizerFlags(1).Expand());
    opacitysizer->Add(mOpacitySpin, wxSizerFlags(0).Right());
    opacitypanel->SetSizer(opacitysizer);
    addOption(_("Opacity"), opacitypanel);

    wxPanel* rotationpanel = new wxPanel(this);
    wxBoxSizer* rotationsizer = new wxBoxSizer(wxHORIZONTAL);
    mRotationSlider = new wxSlider(rotationpanel,wxID_ANY, 1 * model::Constants::sRotationPrecisionFactor, model::Constants::sRotationMin, model::Constants::sRotationMax);
    mRotationSlider->SetPageSize(model::Constants::sRotationPageSize);
    mRotationSpin = new wxSpinCtrlDouble(rotationpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mRotationSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mRotationSpin->SetDigits(model::Constants::sRotationPrecision);
    mRotationSpin->SetValue(0); // No rotation
    mRotationSpin->SetRange(
        static_cast<double>(model::Constants::sRotationMin) / static_cast<double>(model::Constants::sRotationPrecisionFactor),
        static_cast<double>(model::Constants::sRotationMax) / static_cast<double>(model::Constants::sRotationPrecisionFactor));
    mRotationSpin->SetIncrement(sRotationIncrement);
    rotationsizer->Add(mRotationSlider, wxSizerFlags(1).Expand());
    rotationsizer->Add(mRotationSpin, wxSizerFlags(0).Right());
    rotationpanel->SetSizer(rotationsizer);
    addOption(_("Rotation"), rotationpanel);

    wxPanel* scalingpanel = new wxPanel(this);
    wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
    mSelectScaling = new EnumSelector<model::VideoScaling>(scalingpanel, model::VideoScalingConverter::mapToHumanReadibleString, model::VideoScalingNone);
    mSelectScaling->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSlider = new wxSlider(scalingpanel,wxID_ANY, 1 * model::Constants::sScalingPrecisionFactor, model::Constants::sScalingMin, model::Constants::sScalingMax);
    mScalingSlider->SetPageSize(model::Constants::sScalingPageSize);
    mScalingSpin = new wxSpinCtrlDouble(scalingpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mScalingSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mScalingSpin->SetValue(1); // No scaling
    mScalingSpin->SetDigits(model::Constants::sScalingPrecision);
    mScalingSpin->SetRange(
        static_cast<double>(model::Constants::sScalingMin) / static_cast<double>(model::Constants::sScalingPrecisionFactor),
        static_cast<double>(model::Constants::sScalingMax) / static_cast<double>(model::Constants::sScalingPrecisionFactor));
    mScalingSpin->SetIncrement(sScalingIncrement);
    scalingsizer->Add(mSelectScaling, wxSizerFlags(0).Left());
    scalingsizer->Add(mScalingSlider, wxSizerFlags(1).Expand());
    scalingsizer->Add(mScalingSpin, wxSizerFlags(0).Right());
    scalingpanel->SetSizer(scalingsizer);
    addOption(_("Scaling"), scalingpanel);

    wxPanel* alignmentpanel = new wxPanel(this);
    wxBoxSizer* alignmentsizer = new wxBoxSizer(wxHORIZONTAL);
    mSelectAlignment = new EnumSelector<model::VideoAlignment>(alignmentpanel, model::VideoAlignmentConverter::mapToHumanReadibleString, model::VideoAlignmentCustom);
    mSelectAlignment->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    wxStaticText* titleX = new wxStaticText(alignmentpanel, wxID_ANY, _("  X:"), wxDefaultPosition);
    mPositionXSlider = new wxSlider(alignmentpanel, wxID_ANY, 0, 0, 1);
    mPositionXSlider->SetPageSize(sPositionPageSize);
    mPositionXSpin = new wxSpinCtrl(alignmentpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionXSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionXSpin->SetRange(0,1);
    mPositionXSpin->SetValue(0);
    wxStaticText* titleY = new wxStaticText(alignmentpanel, wxID_ANY, _("  Y:"), wxDefaultPosition);
    mPositionYSlider = new wxSlider(alignmentpanel, wxID_ANY, 0, 0, 1);
    mPositionYSlider->SetPageSize(sPositionPageSize);
    mPositionYSpin = new wxSpinCtrl(alignmentpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mPositionYSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mPositionYSpin->SetRange(0,1);
    mPositionYSpin->SetValue(0);
    alignmentsizer->Add(mSelectAlignment, wxSizerFlags(0).Expand());
    alignmentsizer->Add(titleX, wxSizerFlags(0).Expand().Right());
    alignmentsizer->Add(mPositionXSlider, wxSizerFlags(1000).Expand());
    alignmentsizer->Add(mPositionXSpin, wxSizerFlags(0).Expand().Right());
    alignmentsizer->Add(titleY, wxSizerFlags(0).Expand().Right());
    alignmentsizer->Add(mPositionYSlider, wxSizerFlags(1000).Expand());
    alignmentsizer->Add(mPositionYSpin, wxSizerFlags(0).Expand().Right());
    alignmentpanel->SetSizer(alignmentsizer);
    addOption(_("Position"), alignmentpanel);

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

    addBox(sAudio);

    wxPanel* volumepanel = new wxPanel(this);
    wxBoxSizer* volumesizer = new wxBoxSizer(wxHORIZONTAL);
    mVolumeSlider = new wxSlider(volumepanel, wxID_ANY, model::Constants::sDefaultVolume, model::Constants::sMinVolume, model::Constants::sMaxVolume );
    mVolumeSlider->SetPageSize(sVolumePageSize);
    mVolumeSpin = new wxSpinCtrl(volumepanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55,-1));
    mVolumeSpin->SetWindowVariant( wxWINDOW_VARIANT_SMALL );
    mVolumeSpin->SetRange(model::Constants::sMinVolume, model::Constants::sMaxVolume);
    mVolumeSpin->SetValue(model::Constants::sMaxVolume);
    volumesizer->Add(mVolumeSlider, wxSizerFlags(1).Expand());
    volumesizer->Add(mVolumeSpin, wxSizerFlags(0).Right());
    volumepanel->SetSizer(volumesizer);
    addOption(_("Volume (%)"), volumepanel);

    mVolumeSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onVolumeSliderChanged, this);
    mVolumeSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onVolumeSpinChanged, this);

    addBox(sTransition);

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

    setClip(model::IClipPtr()); // Ensures disabling all controls

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

    setClip(model::IClipPtr()); // Ensures Unbind if needed for clip events

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
    if (mClip)
    {
        if (mVideoClip)
        {
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_OPACITY, &DetailsClip::onOpacityChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsClip::onScalingChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, &DetailsClip::onScalingFactorChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_ROTATION, &DetailsClip::onRotationChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, this);
        }
        if (mAudioClip)
        {
            mAudioClip->Unbind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &DetailsClip::onVolumeChanged, this);
        }
        mVideoClip.reset();
        mAudioClip.reset();
        mClip.reset();
    }

    mClip = clip;

    mTransformCommand = 0; // Ensures that a new command is generated for future edits
    mVolumeCommand = 0; // Ensures that a new command is generated for future edits

    if (mClip)
    {
        mTransition = getTypedClip<model::Transition>(clip);
        mVideoClip = getTypedClip<model::VideoClip>(clip);
        mAudioClip = getTypedClip<model::AudioClip>(clip);
        mEmptyClip = getTypedClip<model::EmptyClip>(clip);

        if (mEmptyClip)
        {
            // Can't edit emptyclips
            mClip.reset();
            mEmptyClip.reset();
        }
        else
        {
            if (mTransition || mVideoClip || mAudioClip)
            {
                // For audio and video clips and for transitions, the length can be edited.
                determineClipSizeBounds();
                updateLengthButtons();

                model::IClipPtr link = mClip->getLink();
                pts lClip = mClip->getPerceivedLength();
                pts lLink = link ? link->getPerceivedLength() : 0;
                if (link && lLink !=lClip)
                {
                    mCurrentLength->SetLabel(model::Convert::ptsToHumanReadibleString(lClip) + "/" + model::Convert::ptsToHumanReadibleString(lLink));
                }
                else
                {
                    mCurrentLength->SetLabel(model::Convert::ptsToHumanReadibleString(lClip));
                }
            }

            showBox(sVideo, mVideoClip != nullptr);
            showBox(sAudio, mAudioClip != nullptr);
            // showBox(sTransition, mTransition);
            showBox(sTransition, false);

            if (mVideoClip)
            {

                wxSize originalSize = mVideoClip->getInputSize();
                boost::rational< int > factor = mVideoClip->getScalingFactor();
                boost::rational< int > rotation = mVideoClip->getRotation();
                wxPoint position = mVideoClip->getPosition();
                wxPoint maxpos = mVideoClip->getMaxPosition();
                wxPoint minpos = mVideoClip->getMinPosition();
                int opacity = mVideoClip->getOpacity();
                const double sScalingIncrement = 0.01;

                mOpacitySlider->SetValue(opacity);
                mOpacitySpin->SetValue(opacity);

                mSelectScaling->select(mVideoClip->getScaling());
                double sliderFactor = boost::rational_cast<double>(factor);
                mScalingSlider->SetValue(boost::rational_cast<int>(factor * model::Constants::sScalingPrecisionFactor));
                mScalingSpin->SetValue(sliderFactor);

                double angle = boost::rational_cast<double>(rotation);
                mRotationSlider->SetValue(boost::rational_cast<int>(rotation * model::Constants::sRotationPrecisionFactor));
                mRotationSpin->SetValue(angle);

                mSelectAlignment->select(mVideoClip->getAlignment());
                mPositionXSlider->SetRange(minpos.x,maxpos.x);
                mPositionXSlider->SetValue(position.x);
                mPositionXSpin->SetRange(minpos.x, maxpos.x);
                mPositionXSpin->SetValue(position.x);
                mPositionYSlider->SetRange(minpos.y,maxpos.y);
                mPositionYSlider->SetValue(position.y);
                mPositionYSpin->SetRange(minpos.y, maxpos.y);
                mPositionYSpin->SetValue(position.y);

                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_OPACITY, &DetailsClip::onOpacityChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsClip::onScalingChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, &DetailsClip::onScalingFactorChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_ROTATION, &DetailsClip::onRotationChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, this);
            }
            if (mAudioClip)
            {
                int volume = mAudioClip->getVolume();

                mVolumeSlider->SetValue(volume);
                mVolumeSpin->SetValue(volume);

                mAudioClip->Bind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &DetailsClip::onVolumeChanged, this);
            }
        }
    }

    // Note: disabling a control and then enabling it again can cause extra events (value changed).
    // Therefore this has been placed here, to only dis/enable in the minimal number of cases.
    mOpacitySlider->Enable(mVideoClip != nullptr);
    mOpacitySpin->Enable(mVideoClip != nullptr);
    mSelectScaling->Enable(mVideoClip != nullptr);
    mScalingSlider->Enable(mVideoClip != nullptr);
    mScalingSpin->Enable(mVideoClip != nullptr);
    mRotationSlider->Enable(mVideoClip != nullptr);
    mRotationSpin->Enable(mVideoClip != nullptr);
    mSelectAlignment->Enable(mVideoClip != nullptr);
    mPositionXSlider->Enable(mVideoClip != nullptr);
    mPositionXSpin->Enable(mVideoClip != nullptr);
    mPositionYSlider->Enable(mVideoClip != nullptr);
    mPositionYSpin->Enable(mVideoClip != nullptr);
    mVolumeSlider->Enable(mAudioClip != nullptr);
    mVolumeSpin->Enable(mAudioClip != nullptr);
    Layout();
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

void DetailsClip::onOpacitySliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mOpacitySlider->GetValue());
    makeTransformCommand();
    mTransformCommand->setOpacity(mOpacitySlider->GetValue());
    event.Skip();
}

void DetailsClip::onOpacitySpinChanged(wxSpinEvent& event)
{
    int value = mOpacitySpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    makeTransformCommand();
    mTransformCommand->setOpacity(value);
    event.Skip();
}

void DetailsClip::onScalingChoiceChanged(wxCommandEvent& event)
{
    VAR_INFO(mSelectScaling->getValue());
    makeTransformCommand();
    mTransformCommand->setScaling(mSelectScaling->getValue(), boost::none);
    event.Skip();
}

void DetailsClip::onScalingSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mScalingSlider->GetValue());
    makeTransformCommand();
    boost::rational<int> r(mScalingSlider->GetValue(), model::Constants::sScalingPrecisionFactor);
    mTransformCommand->setScaling(model::VideoScalingCustom, boost::optional< boost::rational< int > >(r));
    event.Skip();
}

void DetailsClip::onScalingSpinChanged(wxSpinDoubleEvent& event)
{
    double value = mScalingSpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    int spinFactor = floor(value * model::Constants::sScalingPrecisionFactor);
    makeTransformCommand();
    boost::rational<int> r(spinFactor, model::Constants::sScalingPrecisionFactor);
    mTransformCommand->setScaling(model::VideoScalingCustom, boost::optional< boost::rational< int > >(r));
    event.Skip();
}

void DetailsClip::onRotationSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mRotationSlider->GetValue());
    makeTransformCommand();
    boost::rational<int> r(mRotationSlider->GetValue(), model::Constants::sRotationPrecisionFactor);
    mTransformCommand->setRotation(r);
    event.Skip();
}

void DetailsClip::onRotationSpinChanged(wxSpinDoubleEvent& event)
{
    double value = mRotationSpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    int spinFactor = floor(value * model::Constants::sRotationPrecisionFactor);
    makeTransformCommand();
    boost::rational<int> r(spinFactor, model::Constants::sRotationPrecisionFactor);
    mTransformCommand->setRotation(r);
    event.Skip();
}

void DetailsClip::onAlignmentChoiceChanged(wxCommandEvent& event)
{
    VAR_INFO(mSelectAlignment->getValue());
    makeTransformCommand();
    mTransformCommand->setAlignment(mSelectAlignment->getValue());
    event.Skip();
}

void DetailsClip::onPositionXSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionXSlider->GetValue());
    makeTransformCommand();
    updateAlignment(true);
    mTransformCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionXSpinChanged(wxSpinEvent& event)
{
    int value = mPositionXSpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    makeTransformCommand();
    updateAlignment(true);
    mTransformCommand->setPosition(wxPoint(value, mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionYSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionYSlider->GetValue());
    makeTransformCommand();
    updateAlignment(false);
    mTransformCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionYSpinChanged(wxSpinEvent& event)
{
    int value = mPositionYSpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    makeTransformCommand();
    updateAlignment(false);
    mTransformCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), value));
    event.Skip();
}

void DetailsClip::onVolumeSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mVolumeSlider->GetValue());
    makeChangeVolumeCommand();
    mVolumeCommand->setVolume(mVolumeSlider->GetValue());
    event.Skip();
}

void DetailsClip::onVolumeSpinChanged(wxSpinEvent& event)
{
    int value = mVolumeSpin->GetValue(); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    makeChangeVolumeCommand();
    mVolumeCommand->setVolume(value);
    event.Skip();
}

void DetailsClip::handleLengthButtonPressed(wxToggleButton* button)
{
    ASSERT_NONZERO(button);
    ASSERT(wxThread::IsMain());
    if (!button->IsEnabled()) { return; }
    pts length = model::Convert::timeToPts(button->GetId());
    VAR_INFO(length);
    ASSERT(mTrimAtEnd.find(length) != mTrimAtEnd.end())(mTrimAtEnd)(length);
    ASSERT(mTrimAtBegin.find(length) != mTrimAtBegin.end())(mTrimAtBegin)(length);

    ::command::Combiner* command = new ::command::Combiner();

    getTimeline().beginTransaction();

    model::IClipPtr clip = mClip;
    pts endtrim = mTrimAtEnd[length];
    pts begintrim = mTrimAtBegin[length];
    bool transition = clip->isA<model::Transition>();
    bool shift = !transition;
    bool error = false;

    if (endtrim != 0)
    {
        ::gui::timeline::command::TrimClip* trimCommand = new command::TrimClip(getSequence(), clip, model::TransitionPtr(), transition ? TransitionEnd : ClipEnd);
        trimCommand->update(endtrim, shift);
        clip = trimCommand->getNewClip();
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
            if (mTransition)
            {
                limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), mTransition, model::IClipPtr(), TransitionBegin, false);
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
            trimCommand->update(begintrim, shift);
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
    preview();
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
    if (selectedclip)
    {
        setClip(selectedclip);
        requestShow(true, selectedclip->getDescription());
    }
    else
    {
        setClip(model::IClipPtr());
        requestShow(false);
    }
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

void DetailsClip::makeTransformCommand()
{
    if (!mTransformCommand || mTransformCommand != model::CommandProcessor::get().GetCurrentCommand())
    {
        // - No transform command has been submitted yet, OR
        // - A transform command was submitted, but
        //   * another command was executed afterwards, OR
        //   * the transform command was undone again.
        // Insert a new transform command into the Undo chain.
        ASSERT(mVideoClip);
        mTransformCommand = new model::ChangeVideoClipTransform(mVideoClip);
        mTransformCommand->submit();
    }
    ASSERT_NONZERO(mTransformCommand);
}

void DetailsClip::makeChangeVolumeCommand()
{
    if (!mVolumeCommand || mVolumeCommand != model::CommandProcessor::get().GetCurrentCommand())
    {
        // - No volume command has been submitted yet, OR
        // - A volume command was submitted, but
        //   * another command was executed afterwards, OR
        //   * the volume command was undone again.
        // Insert a new volume command into the Undo chain.
        ASSERT(mAudioClip);
        mVolumeCommand = new model::ChangeAudioClipVolume(mAudioClip);
        mVolumeCommand->submit();
    }
}

void DetailsClip::preview()
{
    if (!mVideoClip) { return; }

    pts position = getCursor().getLogicalPosition(); // By default, show the frame under the cursor (which is already currently shown, typically)
    if ((position < mVideoClip->getLeftPts()) || (position >= mVideoClip->getRightPts()))
    {
        // The cursor is not positioned under the clip being adjusted. Move the cursor to the middle frame of that clip
        position = mVideoClip->getLeftPts() + mVideoClip->getLength() / 2; // Show the middle frame of the clip
        VAR_DEBUG(position);
        getCursor().setLogicalPosition(position); // ...and move the cursor to that position
    }

    if (mVideoClip->getLength() > 0)
    {
        wxSize s = getPlayer()->getVideoSize();
        boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
        wxMemoryDC dc(*bmp);

        // Fill with black
        dc.SetBrush(Layout::get().PreviewBackgroundBrush);
        dc.SetPen(Layout::get().PreviewBackgroundPen);
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
        getPlayer()->show(bmp);
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
    mTransformCommand->setAlignment(getAlignment());
}

void DetailsClip::determineClipSizeBounds()
{
    ASSERT(mClip);

    model::IClipPtr link = mClip->getLink();

    command::TrimClip::TrimLimit limitsBeginTrim;
    command::TrimClip::TrimLimit limitsEndTrim;
    if (mTransition)
    {
        limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), mTransition, model::IClipPtr(), TransitionBegin, false);
        limitsEndTrim = command::TrimClip::determineBoundaries(getSequence(), mTransition, model::IClipPtr(), TransitionEnd, false);
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
        pts length = model::Convert::timeToPts(button->GetId());
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
                    mTrimAtBegin[length] = (clipPerceivedLength - length) + limitsEndTrim.Min;
                }
                else
                {
                    // Size enlargement
                    mTrimAtEnd[length] = limitsEndTrim.Max;
                    mTrimAtBegin[length] = (clipPerceivedLength - length) - limitsEndTrim.Max;
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
        pts length = model::Convert::timeToPts(button->GetId());
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
