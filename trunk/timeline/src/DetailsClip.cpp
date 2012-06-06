#include "DetailsClip.h"

#include <math.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dcmemory.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include "AudioClip.h"
#include "ChangeVideoClipTransform.h"
#include "CommandProcessor.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "IClip.h"
#include "Layout.h"
#include "Player.h"
#include "Sequence.h"
#include "UtilEnumSelector.h"
#include "UtilLog.h"
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
    :   wxPanel(parent)
    ,   Part(&timeline)
    ,   mClip()
    ,   mVideoClip()
    ,   mAudioClip()
    ,   mTopSizer(0)
    ,   mBoxSizer(0)
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
    ,   mCommand(0)
{
    LOG_INFO;

    VAR_INFO(GetSize());

    mTopSizer = new wxBoxSizer(wxVERTICAL);

    addbox(_("Duration")); // todo handle resizing for clips with a different audio/video length

    addbox(_("Video"));

    wxPanel* opacitypanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mOpacitySlider = new wxSlider(opacitypanel, wxID_ANY, model::Constants::sMaxOpacity, model::Constants::sMinOpacity, model::Constants::sMaxOpacity );
    mOpacitySlider->SetPageSize(sOpacityPageSize);
    mOpacitySpin = new wxSpinCtrl(opacitypanel);
    mOpacitySpin->SetRange(model::Constants::sMinOpacity, model::Constants::sMaxOpacity);
    mOpacitySpin->SetValue(model::Constants::sMaxOpacity);
    opacitysizer->Add(mOpacitySlider);
    opacitysizer->Add(mOpacitySpin);
    opacitypanel->SetSizer(opacitysizer);
    addoption(_("Opacity"), opacitypanel);

    mSelectScaling = new EnumSelector<model::VideoScaling>(this, model::VideoScalingConverter::mapToHumanReadibleString, model::VideoScalingNone);
    addoption(_("Scaling"), mSelectScaling);

    wxPanel* scalingpanel = new wxPanel(this);
    wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
    mScalingSlider = new wxSlider(scalingpanel,wxID_ANY, model::VideoClip::sScalingOriginalSize, model::Constants::sMinScaling, model::Constants::sMaxScaling);
    mScalingSlider->SetPageSize(model::Constants::scalingPageSize);
    mScalingSpin = new wxSpinCtrlDouble(scalingpanel);
    mScalingSpin->SetDigits(model::Constants::scalingPrecision);
    mScalingSpin->SetValue(model::Convert::digitsToFactor(model::VideoClip::sScalingOriginalSize, model::Constants::scalingPrecision));
    mScalingSpin->SetRange(model::Convert::digitsToFactor(model::Constants::sMinScaling, model::Constants::scalingPrecision), model::Convert::digitsToFactor(model::Constants::sMaxScaling, model::Constants::scalingPrecision));
    mScalingSpin->SetIncrement(sScalingIncrement);
    scalingsizer->Add(mScalingSlider);
    scalingsizer->Add(mScalingSpin);
    scalingpanel->SetSizer(scalingsizer);
    addoption(_("Factor"), scalingpanel);

    mSelectAlignment = new EnumSelector<model::VideoAlignment>(this, model::VideoAlignmentConverter::mapToHumanReadibleString, model::VideoAlignmentCustom);
    addoption(_("Alignment"), mSelectAlignment);

    wxPanel* positionxpanel = new wxPanel(this);
    wxBoxSizer* positionxsizer = new wxBoxSizer(wxHORIZONTAL);
    mPositionXSlider = new wxSlider(positionxpanel, wxID_ANY, 0, 0, 1);
    mPositionXSlider->SetPageSize(sPositionPageSize);
    mPositionXSpin = new wxSpinCtrl(positionxpanel);
    mPositionXSpin->SetRange(0,1);
    mPositionXSpin->SetValue(0);
    positionxsizer->Add(mPositionXSlider);
    positionxsizer->Add(mPositionXSpin);
    positionxpanel->SetSizer(positionxsizer);
    addoption(_("X position"), positionxpanel);

    wxPanel* positionypanel = new wxPanel(this);
    wxBoxSizer* positionysizer = new wxBoxSizer(wxHORIZONTAL);
    mPositionYSlider = new wxSlider(positionypanel, wxID_ANY, 0, 0, 1);
    mPositionYSlider->SetPageSize(sPositionPageSize);
    mPositionYSpin = new wxSpinCtrl(positionypanel);
    mPositionYSpin->SetRange(0,1);
    mPositionYSpin->SetValue(0);
    positionysizer->Add(mPositionYSlider);
    positionysizer->Add(mPositionYSpin);
    positionypanel->SetSizer(positionysizer);
    addoption(_("Y position"), positionypanel);

    Bind(wxEVT_SHOW, &DetailsClip::onShow, this);
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

    addbox(_("Audio"));
    addoption(_("test"), new wxButton(this,wxID_ANY,_("label")));

    setClip(model::IClipPtr()); // Ensures disabling all controls

    mTopSizer->AddStretchSpacer();
    SetSizerAndFit(mTopSizer);
    VAR_INFO(GetSize());
}

DetailsClip::~DetailsClip()
{
    setClip(model::IClipPtr()); // Ensures Unbind if needed for clip events
    Unbind(wxEVT_SHOW, &DetailsClip::onShow, this);
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
    if (mClip == clip) return; // Avoid useless updating
    if (mClip)
    {
        if (mVideoClip)
        {
            mOpacitySlider->Disable();
            mOpacitySpin->Disable();
            mSelectScaling->Disable();
            mScalingSlider->Disable();
            mScalingSpin->Disable();
            mSelectAlignment->Disable();
            mPositionXSlider->Disable();
            mPositionXSpin->Disable();
            mPositionYSlider->Disable();
            mPositionYSpin->Disable();

            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_OPACITY, &DetailsClip::onOpacityChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsClip::onScalingChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGDIGITS, &DetailsClip::onScalingDigitsChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, this);
            mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, this);
            mVideoClip.reset();
        }
        mAudioClip.reset();
        mClip.reset();
    }

    mClip = clip;
    mCommand = 0; // Ensures that a new command is generated for future edits

    if (mClip)
    {
        mVideoClip = getTypedClip<model::VideoClip>(clip);
        mAudioClip = getTypedClip<model::AudioClip>(clip);

        if (mVideoClip)
        {
            wxSize originalSize = mVideoClip->getInputSize();
            int factor = mVideoClip->getScalingDigits();
            wxPoint position = mVideoClip->getPosition();
            wxPoint maxpos = mVideoClip->getMaxPosition();
            wxPoint minpos = mVideoClip->getMinPosition();
            int opacity = mVideoClip->getOpacity();
            const double sScalingIncrement = 0.01;

            mOpacitySlider->SetValue(opacity);
            mOpacitySpin->SetValue(opacity);

            mSelectScaling->select(mVideoClip->getScaling());
            mScalingSlider->SetValue(factor);
            mScalingSpin->SetValue(model::Convert::digitsToFactor(factor, model::Constants::scalingPrecision));

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
            mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGDIGITS, &DetailsClip::onScalingDigitsChanged, this);
            mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, this);
            mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, this);
            mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, this);
            mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, this);

            mOpacitySlider->Enable();
            mOpacitySpin->Enable();
            mSelectScaling->Enable();
            mScalingSlider->Enable();
            mScalingSpin->Enable();
            mSelectAlignment->Enable();
            mPositionXSlider->Enable();
            mPositionXSpin->Enable();
            mPositionYSlider->Enable();
            mPositionYSpin->Enable();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onShow(wxShowEvent& event)
{
    if (!event.IsShown())
    {
        setClip(model::IClipPtr());
    }
    event.Skip();
}

void DetailsClip::onOpacitySliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mOpacitySlider->GetValue());
    makeCommand();
    mCommand->setOpacity(mOpacitySlider->GetValue());
    event.Skip();
}

void DetailsClip::onOpacitySpinChanged(wxSpinEvent& event)
{
    VAR_INFO(event.GetValue());
    makeCommand();
    mCommand->setOpacity(event.GetValue());
    event.Skip();
}

void DetailsClip::onScalingChoiceChanged(wxCommandEvent& event)
{
    LOG_INFO;
    makeCommand();
    mCommand->setScaling(mSelectScaling->getValue(), boost::none);
    event.Skip();
}

void DetailsClip::onScalingSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mScalingSlider->GetValue());
    makeCommand();
    mCommand->setScaling(model::VideoScalingCustom, boost::optional<int>(mScalingSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onScalingSpinChanged(wxSpinDoubleEvent& event)
{
    VAR_INFO(event.GetValue());
    int value = model::Convert::factorToDigits(event.GetValue(), model::Constants::scalingPrecision);
    makeCommand();
    mCommand->setScaling(model::VideoScalingCustom, boost::optional<int>(value));
    event.Skip();
}

void DetailsClip::onAlignmentChoiceChanged(wxCommandEvent& event)
{
    LOG_INFO;
    makeCommand();
    mCommand->setAlignment(mSelectAlignment->getValue());
    event.Skip();
}

void DetailsClip::onPositionXSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionXSlider->GetValue());
    makeCommand();
    updateAlignment(true);
    mCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionXSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(event.GetValue());
    makeCommand();
    updateAlignment(true);
    mCommand->setPosition(wxPoint(event.GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionYSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionYSlider->GetValue());
    makeCommand();
    updateAlignment(false);
    mCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
    event.Skip();
}

void DetailsClip::onPositionYSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(event.GetValue());
    makeCommand();
    updateAlignment(false);
    mCommand->setPosition(wxPoint(mPositionXSlider->GetValue(), event.GetValue()));
    event.Skip();
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

void DetailsClip::onScalingDigitsChanged(model::EventChangeVideoClipScalingDigits& event)
{
    mScalingSpin->SetValue(model::Convert::digitsToFactor(event.getValue(),model::Constants::scalingPrecision));
    mScalingSlider->SetValue(event.getValue());
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
// TEST
//////////////////////////////////////////////////////////////////////////

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

void DetailsClip::makeCommand()
{
    if (!mCommand || mCommand != model::CommandProcessor::get().GetCurrentCommand())
    {
        // - No command has been submitted yet, OR
        // - A command was submitted, but another command was executed afterwards, OR
        // - The command was undone again.
        // Insert a new command into the Undo chain.
        mCommand = new model::ChangeVideoClipTransform(mVideoClip);
        mCommand->submit();
    }
    ASSERT_NONZERO(mCommand);
}

void DetailsClip::addbox(const wxString& name)
{
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 0 );
}

void DetailsClip::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 2);
    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 0);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP);
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
        getCursor().setLogicalPosition(position); // ...and move the cursor to that position
    }

    if (videoclip->getLength() > 0)
    {
        wxSize s = getPlayer()->getVideoSize();
        boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
        wxMemoryDC dc(*bmp);
        videoclip->moveTo(0);

        // Fill with black
        dc.SetBrush(Layout::sPreviewBackgroundBrush);
        dc.SetPen(Layout::sPreviewBackgroundPen);
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
    mCommand->setAlignment(getAlignment());
}

}} // namespace