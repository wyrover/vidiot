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

#include "TransitionParameterDirection4.h"

#include "Config.h"
#include "UtilException.h"

namespace model {

IMPLEMENTENUM(Direction4);

std::map<Direction4, wxString> Direction4Converter::getMapToHumanReadibleString()
{
    return
    {
        { Direction4TopToBottom, _("Top to bottom") },
        { Direction4RightToLeft, _("Right to left") },
        { Direction4BottomToTop, _("Bottom to top") },
        { Direction4LeftToRight, _("Left to right") },
    };
}

wxString TransitionParameterDirection4::sParameterDirection4{ "Direction4" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterDirection4::TransitionParameterDirection4()
    : TransitionParameter()
    , mValue{ Direction4LeftToRight }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection4::TransitionParameterDirection4(const Direction4& direction)
    : TransitionParameter()
    , mValue{ direction }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection4::TransitionParameterDirection4(const TransitionParameterDirection4& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection4* TransitionParameterDirection4::clone() const
{
    return new TransitionParameterDirection4(static_cast<const TransitionParameterDirection4&>(*this));
}

TransitionParameterDirection4::~TransitionParameterDirection4()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterDirection4::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterDirection4> typed{ boost::dynamic_pointer_cast<TransitionParameterDirection4>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterDirection4::makeWidget(wxWindow *parent)
{
    ASSERT_EQUALS(mControl, 0);
    mControl = new Direction4Selector(parent, Direction4Converter::getMapToHumanReadibleString(), mValue);
    mControl->Bind(wxEVT_CHOICE, &TransitionParameterDirection4::onDirection, this);
    return mControl;
}
void TransitionParameterDirection4::destroyWidget()
{
    ASSERT_DIFFERS(mControl, 0);
    mControl->Unbind(wxEVT_CHOICE, &TransitionParameterDirection4::onDirection, this);
    mControl->Destroy();
    mControl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

Direction4 TransitionParameterDirection4::getValue() const 
{ 
    return mValue; 
}

void TransitionParameterDirection4::setValue(Direction4 value) 
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

void TransitionParameterDirection4::onDirection(wxCommandEvent& event)
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

std::ostream& operator<<(std::ostream& os, const TransitionParameterDirection4& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterDirection4::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterDirection4::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterDirection4::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterDirection4)
