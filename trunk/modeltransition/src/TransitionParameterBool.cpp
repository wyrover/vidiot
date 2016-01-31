// Copyright 2016 Eric Raijmakers.
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

#include "TransitionParameterBool.h"

#include "UtilException.h"

namespace model {

wxString TransitionParameterBool::sParameterSoftenEdges{ "softenedges" };
wxString TransitionParameterBool::sParameterInversed{ "inversed" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterBool::TransitionParameterBool(bool value)
    : TransitionParameter()
    , mValue{ value }
{
}

TransitionParameterBool::TransitionParameterBool(const TransitionParameterBool& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
{
}

TransitionParameterBool* TransitionParameterBool::clone() const
{
    return new TransitionParameterBool(static_cast<const TransitionParameterBool&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterBool::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterBool> typed{ boost::dynamic_pointer_cast<TransitionParameterBool>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterBool::makeWidget(wxWindow *parent)
{
    ASSERT_ZERO(mCheck);

    mCheck = new wxCheckBox(parent, wxID_ANY, "");
    mCheck->SetValue(mValue);
    mCheck->Bind(wxEVT_CHECKBOX, &TransitionParameterBool::onCheck, this);

    return mCheck;
}

void TransitionParameterBool::destroyWidget()
{
    mCheck->Unbind(wxEVT_CHECKBOX, &TransitionParameterBool::onCheck, this);

    ASSERT_NONZERO(mCheck);

    mCheck->Destroy();
    mCheck = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool TransitionParameterBool::getValue() const 
{ 
    return mValue; 
}

void TransitionParameterBool::setValue(bool value) 
{ 
    if (value != mValue)
    {
        VAR_INFO(value);
        mValue = value;
        if (mCheck != nullptr)
        {
            mCheck->SetValue(mValue);
        }
        signalUpdate();
    }
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterBool::onCheck(wxCommandEvent& event)
{
    CatchExceptions([this]
    {
        setValue(mCheck->GetValue());
    });
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterBool& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterBool::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterBool::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterBool::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterBool)
