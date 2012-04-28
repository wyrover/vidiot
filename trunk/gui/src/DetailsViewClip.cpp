//#include "DetailsViewClip.h"
//
//#include <boost/foreach.hpp>
//#include <wx/button.h>
//#include <wx/stattext.h>
//#include <wx/gbsizer.h>
//#include <wx/sizer.h>
//#include <wx/textctrl.h>
//#include <wx/choice.h>
//#include "IClip.h"
//#include "VideoClip.h"
//#include "UtilEnumSelector.h"
//#include "VideoClipEvent.h"
//#include "CommandProcessor.h"
//#include "AudioClip.h"
//#include "ChangeVideoClipTransform.h"
//#include "UtilLog.h"
//#include "UtilLogWxwidgets.h"
//#include <wx/statbox.h>
//
//namespace gui {
//
//template <class TARGET>
//boost::shared_ptr<TARGET> getTypedClip(model::IClipPtr clip)
//{
//    if (clip->isA<TARGET>())
//    {
//        return boost::dynamic_pointer_cast<TARGET>(clip);
//    }
//    else
//    {
//        return boost::dynamic_pointer_cast<TARGET>(clip->getLink());
//    }
//    return boost::shared_ptr<TARGET>();
//}
//
////////////////////////////////////////////////////////////////////////////
//// INITIALIZATION
////////////////////////////////////////////////////////////////////////////
//
//DetailsViewClip::DetailsViewClip(wxWindow* parent, model::IClipPtr clip)
//    :   wxPanel(parent)
//    ,   mClip(clip)
//    ,   mVideoClip(getTypedClip<model::VideoClip>(clip))
//    ,   mAudioClip(getTypedClip<model::AudioClip>(clip))
//    ,   mTopSizer(0)
//    ,   mBoxSizer(0)
//    ,   mSelectScaling(0)
//    ,   mScalingSlider(0)
//    ,   mScalingSpin(0)
//    ,   mSelectAlignment(0)
//    ,   mOffsetXSpin(0)
//    ,   mOffsetXSlider(0)
//    ,   mOffsetYSpin(0)
//    ,   mOffsetYSlider(0)
//    ,   mCommand(0)
//{
//    LOG_INFO;
//
//    VAR_INFO(GetSize());
//
//    mTopSizer = new wxBoxSizer(wxVERTICAL);
//
//    if (mVideoClip)
//    {
//        addbox(_("Video"));
//
//        wxSize originalSize = mVideoClip->getInputSize();
//        wxSize scaledSize = mVideoClip->getSize();
//        wxRect regionOfInterest = mVideoClip->getRegionOfInterest();
//        double factor = mVideoClip->getScalingFactor();
//        int offsetx = -regionOfInterest.x;
//        int offsety = -regionOfInterest.y;
//
//        mSelectScaling = new EnumSelector<model::VideoScaling>(this, model::VideoScalingConverter::mapToHumanReadibleString, mVideoClip->getScaling());
//        addoption(_("Scaling"), mSelectScaling);
//
//        wxPanel* scalingpanel = new wxPanel(this);
//        wxBoxSizer* scalingsizer = new wxBoxSizer(wxHORIZONTAL);
//
//        static const double minscaling = 0.01;
//        static const int maxscaling = 10;
//
//        static const int sliderFactor = 100; // Each increment of 1 on the slider means 1/100 th increase of scaling.
//        mScalingSlider = new wxSlider(scalingpanel,wxID_ANY, factor * sliderFactor, minscaling * sliderFactor, maxscaling * sliderFactor);
//        mScalingSpin = new wxSpinCtrlDouble(scalingpanel);
//        mScalingSpin->SetValue(factor);
//        mScalingSpin->SetRange(minscaling,maxscaling);
//        mScalingSpin->SetIncrement(0.01);
//        scalingsizer->Add(mScalingSlider);
//        scalingsizer->Add(mScalingSpin);
//        scalingpanel->SetSizer(scalingsizer);
//        addoption(_("Factor"), scalingpanel);
//
//        mSelectAlignment = new EnumSelector<model::VideoAlignment>(this, model::VideoAlignmentConverter::mapToHumanReadibleString, mVideoClip->getAlignment());
//        addoption(_("Alignment"), mSelectAlignment);
//
//        wxString summary = wxString::Format("(%dx%d)->(%dx%d)->(%d,%d)(%d,%d)",
//            originalSize.x, originalSize.y,
//            scaledSize.x, scaledSize.y,
//            regionOfInterest.GetX(), regionOfInterest.GetY(), regionOfInterest.GetWidth(), regionOfInterest.GetHeight());
//        wxStaticText* mSummary = new wxStaticText(this, wxID_ANY, summary );
//        addoption(_("Summary"), mSummary);
//
//        wxPanel* offsetxpanel = new wxPanel(this);
//        wxBoxSizer* offsetxsizer = new wxBoxSizer(wxHORIZONTAL);
//        mOffsetXSlider = new wxSlider(offsetxpanel, wxID_ANY, offsetx, -1000, 1000);
//        mOffsetXSpin = new wxSpinCtrl(offsetxpanel);
//        mOffsetXSlider->SetValue(offsetx);
//        mOffsetXSlider->SetRange(-1000,1000);
//        offsetxsizer->Add(mOffsetXSlider);
//        offsetxsizer->Add(mOffsetXSpin);
//        offsetxpanel->SetSizer(offsetxsizer);
//        addoption(_("X offset"), offsetxpanel);
//
//        wxPanel* offsetypanel = new wxPanel(this);
//        wxBoxSizer* offsetysizer = new wxBoxSizer(wxHORIZONTAL);
//        mOffsetYSlider = new wxSlider(offsetypanel, wxID_ANY, offsety, -1000, 1000);
//        mOffsetYSpin = new wxSpinCtrl(offsetypanel);
//        mOffsetYSlider->SetValue(offsety);
//        mOffsetYSlider->SetRange(-1000,1000);
//        offsetysizer->Add(mOffsetYSlider);
//        offsetysizer->Add(mOffsetYSpin);
//        offsetypanel->SetSizer(offsetysizer);
//        addoption(_("Y offset"), offsetypanel);
//
//        mSelectScaling->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsViewClip::onScalingChoiceChanged, this);
//        mScalingSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsViewClip::onScalingSliderChanged, this);
//        mScalingSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsViewClip::onScalingSpinChanged, this);
//        mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsViewClip::onScalingChanged, this);
//        mVideoClip->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, &DetailsViewClip::onScalingFactorChanged, this);
//    }
//
//    if (mAudioClip)
//    {
//        addbox(_("Audio"));
//        addoption(_("test"), new wxButton(this,wxID_ANY,_("label")));
//    }
//
//    mTopSizer->AddStretchSpacer();
//    SetSizerAndFit(mTopSizer);
//    VAR_INFO(GetSize());
//}
//
//DetailsViewClip::~DetailsViewClip()
//{
//    if (mVideoClip)
//    {
//        mSelectScaling->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DetailsViewClip::onScalingChoiceChanged, this);
//        mScalingSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &DetailsViewClip::onScalingSliderChanged, this);
//        mScalingSpin->Unbind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &DetailsViewClip::onScalingSpinChanged, this);
//        mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsViewClip::onScalingChanged, this);
//        mVideoClip->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, &DetailsViewClip::onScalingFactorChanged, this);
//    }
//    if (mAudioClip)
//    {
//    }
//}
//
////////////////////////////////////////////////////////////////////////////
//// GUI EVENTS
////////////////////////////////////////////////////////////////////////////
//
//void DetailsViewClip::onScalingChoiceChanged(wxCommandEvent& event)
//{
//    LOG_INFO;
//
//    makeCommand();
//    mCommand->setScaling(mSelectScaling->getValue(), boost::none);
//    event.Skip();
//}
//
//void DetailsViewClip::onScalingSliderChanged(wxCommandEvent& event)
//{
//    VAR_INFO(mScalingSlider->GetValue());
//    double value = mScalingSlider->GetValue() / 100.0;
//
//    makeCommand();
//    mCommand->setScaling(model::VideoScalingCustom, boost::optional<double>(value));
//    event.Skip();
//}
//
//void DetailsViewClip::onScalingSpinChanged(wxSpinDoubleEvent& event)
//{
//    VAR_INFO(event.GetValue());
//    double value = event.GetValue();
//    makeCommand();
//    mCommand->setScaling(model::VideoScalingCustom, boost::optional<double>(value));
//    event.Skip();
//}
//
////////////////////////////////////////////////////////////////////////////
//// PROJECT EVENTS
////////////////////////////////////////////////////////////////////////////
//
//void DetailsViewClip::onScalingChanged(model::EventChangeVideoClipScaling& event)
//{
//    mSelectScaling->select(event.getValue());
//    event.Skip();
//}
//
//void DetailsViewClip::onScalingFactorChanged(model::EventChangeVideoClipScalingFactor& event)
//{
//    mScalingSpin->SetValue(event.getValue());
//    mScalingSlider->SetValue(event.getValue() * 100);
//    event.Skip();
//}
//
////////////////////////////////////////////////////////////////////////////
//// HELPER METHODS
////////////////////////////////////////////////////////////////////////////
//
//void DetailsViewClip::makeCommand()
//{
//    if (!mCommand || mCommand != model::CommandProcessor::get().GetCurrentCommand())
//    {
//        // No command has been submitted yet, or a command was submitted, but another command
//        // was executed afterwards. Insert a new command into the Undo chain.
//        mCommand = new model::ChangeVideoClipTransform(mVideoClip);
//        mCommand->submit();
//    }
//    ASSERT_NONZERO(mCommand);
//}
//
//void DetailsViewClip::addbox(const wxString& name)
//{
//    ASSERT(mTopSizer);
//    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
//    mTopSizer->Add(mBoxSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 0 );
//}
//
//void DetailsViewClip::addoption(const wxString& name, wxWindow* widget)
//{
//    ASSERT(mBoxSizer);
//    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
//    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 2);
//    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 0);
//    hSizer->Add(5, 5, 1, wxALL, 0);
//    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP);
//}
//} // namespace