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

#include "TransitionParameterDirection.h"

#include "Config.h"
#include "UtilException.h"

namespace model {

IMPLEMENTENUM(Direction);

std::map<Direction, wxString> DirectionConverter::getMapToHumanReadibleString()
{
    return
    {
        { DirectionTopLeftToBottomRight, _("Top left to bottom right") },
        { DirectionTopToBottom, _("Top to bottom") },
        { DirectionTopRightToBottomLeft, _("Top right to bottom left") },
        { DirectionRightToLeft, _("Right to left") },
        { DirectionBottomRightToTopLeft, _("Bottom right to top left") },
        { DirectionBottomToTop, _("Bottom to top") },
        { DirectionBottomLeftToTopRight, _("Bottom left to top right") },
        { DirectionLeftToRight, _("Left to right") },
    };
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterDirection::TransitionParameterDirection()
    : TransitionParameter()
    , mValue{ DirectionLeftToRight }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection::TransitionParameterDirection(const Direction& direction)
    : TransitionParameter()
    , mValue{ direction }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection::TransitionParameterDirection(const TransitionParameterDirection& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection* TransitionParameterDirection::clone() const
{
    return new TransitionParameterDirection(static_cast<const TransitionParameterDirection&>(*this));
}

TransitionParameterDirection::~TransitionParameterDirection()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterDirection::copyValue(TransitionParameterPtr other)
{
    auto typed{ boost::dynamic_pointer_cast<TransitionParameterDirection>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterDirection::makeWidget(wxWindow *parent) 
{
    ASSERT_EQUALS(mControl, 0);
    mControl = new DirectionSelector(parent, DirectionConverter::getMapToHumanReadibleString(), mValue);
    mControl->Bind(wxEVT_CHOICE, &TransitionParameterDirection::onDirection, this);
    return mControl;
}
void TransitionParameterDirection::destroyWidget() 
{
    ASSERT_DIFFERS(mControl, 0);
    mControl->Unbind(wxEVT_CHOICE, &TransitionParameterDirection::onDirection, this);
    mControl->Destroy();
    mControl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterDirection::onDirection(wxCommandEvent& event)
{
    Direction value{ mControl->getValue() };
    VAR_INFO(value);
    CatchExceptions([this, value]
    {
        if (mValue != value)
        {
            signalUpdate([this, value] { mValue = value; });
        }
    });
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterDirection& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterDirection::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterDirection::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterDirection::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterDirection)
