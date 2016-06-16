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

#include "TransitionParameterDouble.h"

#include "UtilException.h"

namespace model {

wxString TransitionParameterDouble::sParameterScaling{ "scaling" };

static constexpr int sPrecision{ 2 };
const int sPrecisionFactor{ static_cast<int>(pow(10.0, sPrecision)) }; ///< 10^sPrecision
const int sPageSize{ sPrecisionFactor / 10 }; // 0.1
constexpr double sIncrement{ 0.01 };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterDouble::TransitionParameterDouble()
    : TransitionParameter()
    , mValue{ 0.0 }
    , mMin{ 0.0 }
    , mMax{ 0.0 }
{
    VAR_DEBUG(*this);
}

TransitionParameterDouble::TransitionParameterDouble(double value, double min, double max)
    : TransitionParameter()
    , mValue{ value }
    , mMin{ min }
    , mMax{ max }
{
    VAR_DEBUG(*this);
}

TransitionParameterDouble::TransitionParameterDouble(const TransitionParameterDouble& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
    , mMin{ other.mMin }
    , mMax{ other.mMax }
{
    VAR_DEBUG(*this);
}

TransitionParameterDouble* TransitionParameterDouble::clone() const
{
    return new TransitionParameterDouble(static_cast<const TransitionParameterDouble&>(*this));
}

TransitionParameterDouble::~TransitionParameterDouble()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterDouble::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterDouble> typed{ boost::dynamic_pointer_cast<TransitionParameterDouble>(other) };
    if (typed && 
        typed->getValue() >= mMin && 
        typed->getValue() <= mMax)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterDouble::makeWidget(wxWindow *parent)
{
    ASSERT_ZERO(mPanel);
    ASSERT_ZERO(mSlider);
    ASSERT_ZERO(mSpin);

    mPanel = new wxPanel(parent);
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    mSlider = new wxSlider(mPanel, wxID_ANY, mValue * sPrecisionFactor, mMin * sPrecisionFactor, mMax * sPrecisionFactor);
    mSlider->SetPageSize(sPageSize);
    mSlider->SetToolTip(getToolTip());
    mSpin = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(55, -1));
    mSpin->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
    mSpin->SetDigits(sPrecision);
    mSpin->SetValue(mValue);
    mSpin->SetRange(mMin, mMax);
    mSpin->SetIncrement(sIncrement);
    mSpin->SetToolTip(getToolTip());
    sizer->Add(mSlider, wxSizerFlags(1000).Expand());
    sizer->Add(mSpin, wxSizerFlags(0).Expand());
    mPanel->SetSizer(sizer);
    mPanel->SetToolTip(getToolTip());

    mSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &TransitionParameterDouble::onSlider, this);
    mSpin->Bind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &TransitionParameterDouble::onSpin, this);

    return mPanel;
}
void TransitionParameterDouble::destroyWidget()
{
    mSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &TransitionParameterDouble::onSlider, this);
    mSpin->Unbind(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED, &TransitionParameterDouble::onSpin, this);

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

double TransitionParameterDouble::getValue() const 
{ 
    return mValue; 
}

void TransitionParameterDouble::setValue(double value)
{
    if (mValue != value)
    {
        VAR_INFO(value);
        mValue = value; 
        if (mSlider != nullptr)
        {
            mSlider->SetValue(value * sPrecisionFactor);
        }
        if (mSpin != nullptr)
        {
            mSpin->SetValue(value);
        }
        signalUpdate();
    }
}

double TransitionParameterDouble::getMin() const
{
    return mMin;
}

double TransitionParameterDouble::getMax() const
{
    return mMax;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterDouble::onSlider(wxCommandEvent& event)
{
    CatchExceptions([this]
    {
        rational64 r(mSlider->GetValue(), sPrecisionFactor);
        setValue(boost::rational_cast<double>(r));
    });
    event.Skip();
}

void TransitionParameterDouble::onSpin(wxSpinDoubleEvent& event)
{
    CatchExceptions([this]
    {
        // NOT: use event.GetValue() -- The event's value may be outside the range boundaries
        int spinFactor{ narrow_cast<int>(floor(std::round(mSpin->GetValue() * sPrecisionFactor))) }; // This construct ensures that adding +0.01 is not ignored due to rounding issues
        rational64 r(spinFactor, sPrecisionFactor);
        setValue(boost::rational_cast<double>(r));
    });
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterDouble& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterDouble::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterDouble::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterDouble::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterDouble)
