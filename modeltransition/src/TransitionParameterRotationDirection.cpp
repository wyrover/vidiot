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

#include "TransitionParameterRotationDirection.h"

#include "Config.h"
#include "UtilException.h"

namespace model {

IMPLEMENTENUM(RotationDirection);

std::map<RotationDirection, wxString> RotationDirectionConverter::getMapToHumanReadibleString()
{
    return
    {
        { RotationDirectionClockWise, _("Clockwise")},
        { RotationDirectionCounterClockWise, _("Counterclockwise") },
    };
}

wxString TransitionParameterRotationDirection::sParameterRotationDirection{ "rotationdirection" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterRotationDirection::TransitionParameterRotationDirection()
    : TransitionParameter()
    , mValue{ RotationDirectionClockWise }
{
    VAR_DEBUG(*this);
}

TransitionParameterRotationDirection::TransitionParameterRotationDirection(const RotationDirection& direction)
    : TransitionParameter()
    , mValue{ direction }
{
    VAR_DEBUG(*this);
}

TransitionParameterRotationDirection::TransitionParameterRotationDirection(const TransitionParameterRotationDirection& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
{
    VAR_DEBUG(*this);
}

TransitionParameterRotationDirection* TransitionParameterRotationDirection::clone() const
{
    return new TransitionParameterRotationDirection(static_cast<const TransitionParameterRotationDirection&>(*this));
}

TransitionParameterRotationDirection::~TransitionParameterRotationDirection()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterRotationDirection::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterRotationDirection> typed{ boost::dynamic_pointer_cast<TransitionParameterRotationDirection>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterRotationDirection::makeWidget(wxWindow *parent)
{
    ASSERT_EQUALS(mControl, 0);
    mControl = new RotationDirectionSelector(parent, RotationDirectionConverter::getMapToHumanReadibleString(), mValue);
    mControl->Bind(wxEVT_CHOICE, &TransitionParameterRotationDirection::onRotationDirection, this);
    return mControl;
}
void TransitionParameterRotationDirection::destroyWidget()
{
    ASSERT_DIFFERS(mControl, 0);
    mControl->Unbind(wxEVT_CHOICE, &TransitionParameterRotationDirection::onRotationDirection, this);
    mControl->Destroy();
    mControl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

RotationDirection TransitionParameterRotationDirection::getValue() const
{ 
    return mValue; 
}

void TransitionParameterRotationDirection::setValue(RotationDirection value)
{ 
    if (mValue != value)
    {
        mValue = value;
        if (mControl != nullptr)
        {
            VAR_INFO(value);
            mControl->select(value);
        }
        signalUpdate();
    }
}


//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterRotationDirection::onRotationDirection(wxCommandEvent& event)
{
    CatchExceptions([this]
    {
        setValue(mControl->getValue());
    });
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterRotationDirection& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterRotationDirection::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterRotationDirection::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterRotationDirection::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterRotationDirection)
