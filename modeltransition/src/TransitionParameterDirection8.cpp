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

#include "TransitionParameterDirection8.h"

#include "Config.h"
#include "UtilException.h"

namespace model {

IMPLEMENTENUM(Direction8);

std::map<Direction8, wxString> Direction8Converter::getMapToHumanReadibleString()
{
    return
    {
        { Direction8TopLeftToBottomRight, _("Top left to bottom right") },
        { Direction8TopToBottom, _("Top to bottom") },
        { Direction8TopRightToBottomLeft, _("Top right to bottom left") },
        { Direction8RightToLeft, _("Right to left") },
        { Direction8BottomRightToTopLeft, _("Bottom right to top left") },
        { Direction8BottomToTop, _("Bottom to top") },
        { Direction8BottomLeftToTopRight, _("Bottom left to top right") },
        { Direction8LeftToRight, _("Left to right") },
    };
}

wxString TransitionParameterDirection8::sParameterDirection8{ "direction8" };

// static 
Direction8 TransitionParameterDirection8::getInversedDirection(Direction8 direction)
{
    static std::map<Direction8, Direction8> sMap =
    {
        { Direction8TopLeftToBottomRight, Direction8BottomRightToTopLeft },
        { Direction8TopToBottom, Direction8BottomToTop },
        { Direction8TopRightToBottomLeft,Direction8BottomLeftToTopRight },
        { Direction8RightToLeft, Direction8LeftToRight },
        { Direction8BottomRightToTopLeft, Direction8TopLeftToBottomRight },
        { Direction8BottomToTop, Direction8TopToBottom },
        { Direction8BottomLeftToTopRight, Direction8TopRightToBottomLeft },
        { Direction8LeftToRight,Direction8RightToLeft },
    };
    ASSERT_MAP_CONTAINS(sMap, direction);
    return sMap.find(direction)->second;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterDirection8::TransitionParameterDirection8()
    : TransitionParameter()
    , mValue{ Direction8LeftToRight }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection8::TransitionParameterDirection8(const Direction8& direction)
    : TransitionParameter()
    , mValue{ direction }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection8::TransitionParameterDirection8(const TransitionParameterDirection8& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
{
    VAR_DEBUG(*this);
}

TransitionParameterDirection8* TransitionParameterDirection8::clone() const
{
    return new TransitionParameterDirection8(static_cast<const TransitionParameterDirection8&>(*this));
}

TransitionParameterDirection8::~TransitionParameterDirection8()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterDirection8::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterDirection8> typed{ boost::dynamic_pointer_cast<TransitionParameterDirection8>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterDirection8::makeWidget(wxWindow *parent)
{
    ASSERT_EQUALS(mControl, 0);
    mControl = new Direction8Selector(parent, Direction8Converter::getMapToHumanReadibleString(), mValue);
    mControl->Bind(wxEVT_CHOICE, &TransitionParameterDirection8::onDirection, this);
    return mControl;
}
void TransitionParameterDirection8::destroyWidget()
{
    ASSERT_DIFFERS(mControl, 0);
    mControl->Unbind(wxEVT_CHOICE, &TransitionParameterDirection8::onDirection, this);
    mControl->Destroy();
    mControl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

Direction8 TransitionParameterDirection8::getValue() const 
{ 
    return mValue; 
}

void TransitionParameterDirection8::setValue(Direction8 value) 
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

void TransitionParameterDirection8::onDirection(wxCommandEvent& event)
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

std::ostream& operator<<(std::ostream& os, const TransitionParameterDirection8& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterDirection8::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterDirection8::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterDirection8::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterDirection8)
