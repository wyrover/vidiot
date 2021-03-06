// Copyright 2015-2016 Eric Raijmakers.
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

#include "TransitionParameterInt.h"

#include "UtilException.h"

namespace model {

wxString TransitionParameterInt::sParameterAngle{ "angle" };
wxString TransitionParameterInt::sParameterRotations{ "rotations" };
wxString TransitionParameterInt::sParameterBandsCount{ "bandscount" };
wxString TransitionParameterInt::sParameterSoften{ "softenfactor" };
wxString TransitionParameterInt::sParameterBlur{ "blur" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterInt::TransitionParameterInt()
    : TransitionParameter()
    , mValue{ 0 }
    , mMin{ 0 }
    , mMax{ 0 }
{
    VAR_DEBUG(*this);
}

TransitionParameterInt::TransitionParameterInt(int value, int min, int max)
    : TransitionParameter()
    , mValue{ value }
    , mMin{ min }
    , mMax{ max }
{
    VAR_DEBUG(*this);
}

TransitionParameterInt::TransitionParameterInt(const TransitionParameterInt& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
    , mMin{ other.mMin }
    , mMax{ other.mMax }
{
    VAR_DEBUG(*this);
}

TransitionParameterInt* TransitionParameterInt::clone() const
{
    return new TransitionParameterInt(static_cast<const TransitionParameterInt&>(*this));
}

TransitionParameterInt::~TransitionParameterInt()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterInt::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterInt> typed{ boost::dynamic_pointer_cast<TransitionParameterInt>(other) };
    if (typed && 
        typed->getValue() >= mMin && 
        typed->getValue() <= mMax)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterInt::makeWidget(wxWindow *parent)
{
    ASSERT_ZERO(mPanel);
    ASSERT_ZERO(mSlider);
    ASSERT_ZERO(mSpin);

    mPanel = new wxPanel(parent);
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    mSlider = new wxSlider(mPanel, wxID_ANY, mValue, mMin, mMax);
    mSlider->SetPageSize(10);
    mSlider->SetToolTip(getToolTip());
    mSpin = new wxSpinCtrl(mPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
    mSpin->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    mSpin->SetRange(mMin, mMax);
    mSpin->SetValue(mValue);
    mSpin->SetToolTip(getToolTip());
    sizer->Add(mSlider, wxSizerFlags(1000).Expand());
    sizer->Add(mSpin, wxSizerFlags(0).Expand());
    mPanel->SetSizer(sizer);
    mPanel->SetToolTip(getToolTip());

    mSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &TransitionParameterInt::onSlider, this);
    mSpin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TransitionParameterInt::onSpin, this);

    return mPanel;
}
void TransitionParameterInt::destroyWidget()
{
    mSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &TransitionParameterInt::onSlider, this);
    mSpin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TransitionParameterInt::onSpin, this);

    ASSERT_NONZERO(mPanel);
    ASSERT_NONZERO(mSlider);
    ASSERT_NONZERO(mSpin);

    mPanel->Destroy();
    mPanel = nullptr;
    mSlider = nullptr;
    mSpin = nullptr;
}
//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int TransitionParameterInt::getValue() const 
{ 
    return mValue; 
}

void TransitionParameterInt::setValue(int value) 
{
    if (mValue != value)
    {
        VAR_INFO(value);
        if (mSlider != nullptr)
        {
            mSlider->SetValue(value);
        }
        if (mSpin != nullptr)
        {
            mSpin->SetValue(value);
        }
        mValue = value;
    }
    signalUpdate();
}

int TransitionParameterInt::getMin() const
{
    return mMin;
}

int TransitionParameterInt::getMax() const
{
    return mMax;
}


//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterInt::onSlider(wxCommandEvent& event)
{
    CatchExceptions([this]
    {
        setValue(mSlider->GetValue());
    });
    event.Skip();
}

void TransitionParameterInt::onSpin(wxSpinEvent& event)
{
    CatchExceptions([this]
    {
        setValue(mSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    });
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterInt& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterInt::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TransitionParameter);
        ar & BOOST_SERIALIZATION_NVP(mValue);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void TransitionParameterInt::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterInt::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterInt)
