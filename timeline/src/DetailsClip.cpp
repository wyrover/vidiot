// Copyright 2013 Eric Raijmakers.
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
#include "Transition.h"
#include "Trim.h"
#include "TrimClip.h"
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
const int sPositionPageSize = 10;
const int sOpacityPageSize = 10;

DetailsClip::DetailsClip(wxWindow* parent, Timeline& timeline)
    :   DetailsPanel(parent,timeline)
    ,   mClip()
    ,   mVideoClip()
    ,   mAudioClip()
    ,   mCurrentLength()
    ,   mOpacitySlider(0)
    ,   mOpacitySpin(0)
    ,   mSelectScaling(0)
    ,   mScalingSlider(0)
    ,   mScalingSpin(0)
    ,   mSelectAlignment(0)
    ,   mPositionXSpin(0)
    ,   mPositionXSlider(0)
    ,   mPositionYSpin(0)
    ,   mPositionYSlider(0)
    ,   mTransformCommand(0)
    ,   mMinimumLengthWhenBeginTrimming(0)
    ,   mMaximumLengthWhenBeginTrimming(0)
    ,   mMinimumLengthWhenEndTrimming(0)
    ,   mMaximumLengthWhenEndTrimming(0)
{
    VAR_DEBUG(this);

    addBox(_("Duration"));

    mCurrentLength = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxST_NO_AUTORESIZE);
    addOption(_("Current length"), mCurrentLength); // todo only shows length for selected clip not for the link.

    std::list<int> defaultLengths = boost::assign::list_of(500)  (1000) (1500) (2000) (2500) (3000);
    std::list<wxString> labels    = boost::assign::list_of("0.5")("1.0")("1.5")("2.0")("2.5")("3.0");
    wxPanel* lengthbuttonspanel = new wxPanel(this);
    lengthbuttonspanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    auto it = defaultLengths.begin();
    auto itLabel = labels.begin();
    for (auto it = defaultLengths.begin(); it != defaultLengths.end(); ++it, ++itLabel)
    {
        int length = *it;
        // Use the integer as id
        wxToggleButton* button = new wxToggleButton(lengthbuttonspanel, length, *itLabel, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        button->SetToolTip(_("Change the length of the clip to this length. Hold shift when pressing to avoid introducing a black area."));
        lengthbuttonspanel->GetSizer()->Add(button,wxSizerFlags(1));
        button->Bind( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
        mLengthButtons.push_back(button);
    }
    updateLengthButtons();
    addOption(_("Fixed lengths (s)"),lengthbuttonspanel);

    addBox(_("Video"));

    wxPanel* opacitypanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mOpacitySlider = new wxSlider(opacitypanel, wxID_ANY, model::Constants::sMaxOpacity, model::Constants::sMinOpacity, model::Constants::sMaxOpacity );
    mOpacitySlider->SetPageSize(sOpacityPageSize);
    mOpacitySpin = new wxSpinCtrl(opacitypanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75,-1));
    mOpacitySpin->SetRange(model::Constants::sMinOpacity, model::Constants::sMaxOpacity);
    mOpacitySpin->SetValue(model::Constants::sMaxOpacity);
    opacitysizer->Add(mOpacitySlider, wxSizerFlags(1).Expand());
    opacitysizer->Add(mOpacitySpin, wxSizerFlags(0).Right());
    opacitypanel->SetSizer(opacitysizer);
    addOption(_("Opacity"), opacitypanel);

    mSelectScaling = new EnumSelector<model::VideoScaling>(this, model::VideoScalingConverter::mapToHumanReadibleString, model::VideoScalingNone);
    addOption(_("Scaling"), mSelectScaling);

    wxPanel* scalingpanel = new wxPanel(this);
    wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
    mScalingSlider = new wxSlider(scalingpanel,wxID_ANY, 1 * model::Constants::scalingPrecisionFactor, model::Constants::sMinScaling, model::Constants::sMaxScaling);
    mScalingSlider->SetPageSize(model::Constants::scalingPageSize);
    mScalingSpin = new wxSpinCtrlDouble(scalingpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75,-1));
    mScalingSpin->SetDigits(model::Constants::scalingPrecision);
    mScalingSpin->SetValue(1); // No scaling
    mScalingSpin->SetRange(
        static_cast<double>(model::Constants::sMinScaling) / static_cast<double>(model::Constants::scalingPrecisionFactor),
        static_cast<double>(model::Constants::sMaxScaling) / static_cast<double>(model::Constants::scalingPrecisionFactor));
    mScalingSpin->SetIncrement(sScalingIncrement);
    scalingsizer->Add(mScalingSlider, wxSizerFlags(1).Expand());
    scalingsizer->Add(mScalingSpin, wxSizerFlags(0).Right());
    scalingpanel->SetSizer(scalingsizer);
    addOption(_("Factor"), scalingpanel);

    mSelectAlignment = new EnumSelector<model::VideoAlignment>(this, model::VideoAlignmentConverter::mapToHumanReadibleString, model::VideoAlignmentCustom);
    addOption(_("Alignment"), mSelectAlignment);

    wxPanel* positionxpanel = new wxPanel(this);
    wxBoxSizer* positionxsizer = new wxBoxSizer(wxHORIZONTAL);
    mPositionXSlider = new wxSlider(positionxpanel, wxID_ANY, 0, 0, 1);
    mPositionXSlider->SetPageSize(sPositionPageSize);
    mPositionXSpin = new wxSpinCtrl(positionxpanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75,-1));
    mPositionXSpin->SetRange(0,1);
    mPositionXSpin->SetValue(0);
    positionxsizer->Add(mPositionXSlider, wxSizerFlags(1).Expand());
    positionxsizer->Add(mPositionXSpin, wxSizerFlags(0).Right());
    positionxpanel->SetSizer(positionxsizer);
    addOption(_("X position"), positionxpanel);

    wxPanel* positionypanel = new wxPanel(this);
    wxBoxSizer* positionysizer = new wxBoxSizer(wxHORIZONTAL);
    mPositionYSlider = new wxSlider(positionypanel, wxID_ANY, 0, 0, 1);
    mPositionYSlider->SetPageSize(sPositionPageSize);
    mPositionYSpin = new wxSpinCtrl(positionypanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75,-1));
    mPositionYSpin->SetRange(0,1);
    mPositionYSpin->SetValue(0);
    positionysizer->Add(mPositionYSlider, wxSizerFlags(1).Expand());
    positionysizer->Add(mPositionYSpin, wxSizerFlags(0).Right());
    positionypanel->SetSizer(positionysizer);
    addOption(_("Y position"), positionypanel);

    mOpacitySlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onOpacitySliderChanged, this);
    mOpacitySpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onOpacitySpinChanged, this);
    mSelectScaling->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onScalingChoiceChanged, this);
    mScalingSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onScalingSliderChanged, this);
    mScalingSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onScalingSpinChanged, this);
    mSelectAlignment->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onAlignmentChoiceChanged, this);
    mPositionXSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionXSliderChanged, this);
    mPositionXSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionXSpinChanged, this);
    mPositionYSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionYSliderChanged, this);
    mPositionYSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionYSpinChanged, this);

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
    BOOST_FOREACH( wxToggleButton* button, mLengthButtons )
    {
        button->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &DetailsClip::onLengthButtonPressed, this);
    }
    getSelection().Unbind(EVENT_SELECTION_UPDATE, &DetailsClip::onSelectionChanged, this);
    setClip(model::IClipPtr()); // Ensures Unbind if needed for clip events

    mOpacitySlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onOpacitySliderChanged, this);
    mOpacitySpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onOpacitySpinChanged, this);
    mSelectScaling->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onScalingChoiceChanged, this);
    mScalingSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onScalingSliderChanged, this);
    mScalingSpin->Unbind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsClip::onScalingSpinChanged, this);
    mSelectAlignment->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsClip::onAlignmentChoiceChanged, this);
    mPositionXSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionXSliderChanged, this);
    mPositionXSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionXSpinChanged, this);
    mPositionYSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsClip::onPositionYSliderChanged, this);
    mPositionYSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DetailsClip::onPositionYSpinChanged, this);

    Unbind(wxEVT_SHOW, &DetailsClip::onShow, this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr DetailsClip::getClip() const
{
    return mClip;
}

void DetailsClip::setClip(model::IClipPtr clip)
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
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, this);
        }
        mVideoClip.reset();
        mAudioClip.reset();
        mClip.reset();
    }

    mClip = clip;

    mTransformCommand = 0; // Ensures that a new command is generated for future edits

    if (mClip)
    {
        if (mClip->isA<model::Transition>())
        {
            mClip.reset();
        }
        else
        {
            determineClipSizeBounds();
            updateLengthButtons();

            mCurrentLength->SetLabel(model::Convert::ptsToHumanReadibleString(mClip->getLength()) + _(" seconds"));

            mVideoClip = getTypedClip<model::VideoClip>(clip);
            mAudioClip = getTypedClip<model::AudioClip>(clip);
            if (mVideoClip)
            {

                wxSize originalSize = mVideoClip->getInputSize();
                boost::rational< int > factor = mVideoClip->getScalingFactor();
                wxPoint position = mVideoClip->getPosition();
                wxPoint maxpos = mVideoClip->getMaxPosition();
                wxPoint minpos = mVideoClip->getMinPosition();
                int opacity = mVideoClip->getOpacity();
                const double sScalingIncrement = 0.01;

                mOpacitySlider->SetValue(opacity);
                mOpacitySpin->SetValue(opacity);

                mSelectScaling->select(mVideoClip->getScaling());
                double sliderFactor = boost::rational_cast<double>(factor);
                mScalingSlider->SetValue(boost::rational_cast<int>(factor * model::Constants::scalingPrecisionFactor));
                mScalingSpin->SetValue(sliderFactor);

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
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, this);
                mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, this);
            }
        }
    }

    // Note: disabling a control and then enabling it again can cause extra events (value changed).
    // Therefore this has been placed here, to only dis/enable in the minimal number of cases.
    mOpacitySlider->Enable(mVideoClip);
    mOpacitySpin->Enable(mVideoClip);
    mSelectScaling->Enable(mVideoClip);
    mScalingSlider->Enable(mVideoClip);
    mScalingSpin->Enable(mVideoClip);;
    mSelectAlignment->Enable(mVideoClip);
    mPositionXSlider->Enable(mVideoClip);
    mPositionXSpin->Enable(mVideoClip);
    mPositionYSlider->Enable(mVideoClip);
    mPositionYSpin->Enable(mVideoClip);
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
    VAR_INFO(event.GetValue());
    makeTransformCommand();
    mTransformCommand->setOpacity(event.GetValue());
    event.Skip();
}

void DetailsClip::onScalingChoiceChanged(wxCommandEvent& event)
{
    LOG_INFO;
    makeTransformCommand();
    mTransformCommand->setScaling(mSelectScaling->getValue(), boost::none);
    event.Skip();
}

void DetailsClip::onScalingSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mScalingSlider->GetValue());
    makeTransformCommand();
    boost::rational<int> r(mScalingSlider->GetValue(), model::Constants::scalingPrecisionFactor);
    mTransformCommand->setScaling(model::VideoScalingCustom, boost::optional< boost::rational< int > >(r));
    event.Skip();
}

void DetailsClip::onScalingSpinChanged(wxSpinDoubleEvent& event)
{
    VAR_INFO(event.GetValue());
    int spinFactor = floor(event.GetValue() * model::Constants::scalingPrecisionFactor);
    makeTransformCommand();
    boost::rational<int> r(floor(event.GetValue() * model::Constants::scalingPrecisionFactor), model::Constants::scalingPrecisionFactor);
    mTransformCommand->setScaling(model::VideoScalingCustom, boost::optional< boost::rational< int > >(r));
    event.Skip();
}

void DetailsClip::onAlignmentChoiceChanged(wxCommandEvent& event)
{
    LOG_INFO;
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

    VAR_INFO(event.GetValue());
    makeTransformCommand();
    updateAlignment(true);
    mTransformCommand->setPosition(wxPoint(event.GetValue(), mPositionYSlider->GetValue()));
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
    VAR_INFO(event.GetValue());
    makeTransformCommand();
    updateAlignment(false);
    mTransformCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), event.GetValue()));
    event.Skip();
}

void DetailsClip::handleLengthButtonPressed(wxToggleButton* button)
{
    ASSERT_NONZERO(button);
    ASSERT(wxThread::IsMain());
    pts newLength = model::Convert::timeToPts(button->GetId());
    VAR_INFO(newLength);
    ASSERT(mTrimAtEnd.find(newLength) != mTrimAtEnd.end())(mTrimAtEnd)(newLength);
    pts currentLength = mClip->getLength();
    pts trim = newLength - currentLength;
    if (!mTrimAtEnd[newLength])
    {
        trim = -1 * trim;
    }
    ::gui::timeline::command::TrimClip* command = new command::TrimClip(getSequence(), mClip, model::TransitionPtr(), mTrimAtEnd[newLength] ? ClipEnd : ClipBegin);
    command->update(trim, true);
    command->submit();
    // It might be possible that a new length selection button has already been pressed
    // and it's button event is already queued. When that event is handled this new clip
    // must be used.
    setClip(command->getNewClip());
    //NOT: updateLengthButtons(); -- this is automatically done after selecting a new clip

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
    mScalingSlider->SetValue(floor(event.getValue() * model::Constants::scalingPrecisionFactor));
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
    mPositionYSpin->SetRange(mPositionYSpin->GetMax(), event.getValue().y);
    mPositionXSlider->SetRange(mPositionXSlider->GetMin(),event.getValue().x);
    mPositionYSlider->SetRange(mPositionYSlider->GetMax(), event.getValue().y);
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// SELECTION EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onSelectionChanged( timeline::EventSelectionUpdate& event )
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

std::list<wxToggleButton*> DetailsClip::getLengthButtons() const
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

void DetailsClip::preview()
{
    if (!mVideoClip) { return; }
    model::VideoClipPtr videoclip = make_cloned<model::VideoClip>(mVideoClip);

    pts position = getCursor().getLogicalPosition(); // By default, show the frame under the cursor (which is already currently shown, typically)
    if ((position < mVideoClip->getLeftPts()) || (position >= mVideoClip->getRightPts()))
    {
        // The cursor is not positioned under the clip being adjusted. Move the cursor to the middle frame of that clip
        position = mVideoClip->getLeftPts() + mVideoClip->getLength() / 2; // Show the middle frame of the clip
        VAR_ERROR(position);
        getCursor().setLogicalPosition(position); // ...and move the cursor to that position
    }

    if (videoclip->getLength() > 0)
    {
        wxSize s = getPlayer()->getVideoSize();
        boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
        wxMemoryDC dc(*bmp);
        videoclip->moveTo(0);

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
            dc.DrawBitmap(*(compositeFrame->getBitmap()), wxPoint(0,0));
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

    command::TrimClip::TrimLimit limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), mClip, mClip->getLink(), ClipBegin, true);
    command::TrimClip::TrimLimit limitsEndTrim = command::TrimClip::determineBoundaries(getSequence(), mClip, mClip->getLink(), ClipEnd, true);

    // Note that in the code below only one trim operation (either begin OR end) is used for determining the possible new lengths.
    // Reason for this limitation is the fact that all boundaries computation is done taking only one trim operation into account.
    // Particularly, dealing with both a begin and end trim simultaneously make the calculation for the boundaries imposed by
    // 'clips in other track' very difficult.
    mMinimumLengthWhenBeginTrimming = mClip->getLength() + -1 * limitsBeginTrim.Max;
    mMaximumLengthWhenBeginTrimming = mClip->getLength() + -1 * limitsBeginTrim.Min;
    mMinimumLengthWhenEndTrimming   = mClip->getLength() + limitsEndTrim.Min;
    mMaximumLengthWhenEndTrimming   = mClip->getLength() + limitsEndTrim.Max;
    VAR_DEBUG(mMinimumLengthWhenBeginTrimming)(mMaximumLengthWhenBeginTrimming)(mMinimumLengthWhenEndTrimming)(mMaximumLengthWhenEndTrimming);

    // For each possible length, store if it should be achieved by trimming at the beginning or at the end (the default)
    mTrimAtEnd.clear();
    BOOST_FOREACH( wxToggleButton* button, mLengthButtons )
    {
        pts length = model::Convert::timeToPts(button->GetId());
        mTrimAtEnd[length] = (length >= mMinimumLengthWhenEndTrimming && length <= mMaximumLengthWhenEndTrimming);
    }

}

void DetailsClip::updateLengthButtons()
{
    if (!mClip)
    {
        BOOST_FOREACH( wxToggleButton* button, mLengthButtons )
        {
            button->SetValue(false);
            button->Disable();
        }
        return;
    }

    if (!mClip->isA<model::EmptyClip>())
    {
        pts minimumClipLength = std::min(mMinimumLengthWhenBeginTrimming, mMinimumLengthWhenEndTrimming);
        pts maximumClipLength = std::max(mMaximumLengthWhenBeginTrimming, mMaximumLengthWhenEndTrimming);
        ASSERT_MORE_THAN_EQUALS(mClip->getLength(), minimumClipLength);
        ASSERT_LESS_THAN_EQUALS(mClip->getLength(), maximumClipLength);

        BOOST_FOREACH( wxToggleButton* button, mLengthButtons )
        {
            pts length = model::Convert::timeToPts(button->GetId());
            button->SetValue(mClip && mClip->getLength() == length);
            button->Disable();

            if (length >= minimumClipLength && length <= maximumClipLength)
            {
                button->Enable();
            }
        }
    }
}

}} // namespace