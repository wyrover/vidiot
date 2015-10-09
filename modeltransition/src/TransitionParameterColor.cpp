// Copyright 2015 Eric Raijmakers.
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

#include "TransitionParameterColor.h"

#include "UtilBind.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterColor::TransitionParameterColor()
    : TransitionParameter()
    , mColor{ 0, 0, 0 }
    , mControl{ nullptr }
{
    VAR_DEBUG(*this);
}

TransitionParameterColor::TransitionParameterColor(const wxColour& colour)
    : TransitionParameter()
    , mColor{ colour }
    , mControl{ nullptr }
{
    VAR_DEBUG(*this);
}

TransitionParameterColor::TransitionParameterColor(const TransitionParameterColor& other)
    : TransitionParameter(other)
    , mColor{ other.mColor }
    , mControl{ nullptr }
{
    VAR_DEBUG(*this);
}

TransitionParameterColor* TransitionParameterColor::clone() const
{
    return new TransitionParameterColor(static_cast<const TransitionParameterColor&>(*this));
}

TransitionParameterColor::~TransitionParameterColor()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

wxString TransitionParameterColor::getName() 
{ 
    return _("Color"); 
};

wxWindow* TransitionParameterColor::makeWidget(wxWindow *parent) 
{
    ASSERT_EQUALS(mControl, 0);
    mControl = new wxColourPickerCtrl(parent, wxID_ANY, mColor, wxDefaultPosition, wxDefaultSize, wxCLRP_USE_TEXTCTRL | wxCLRP_SHOW_LABEL);
    BindAndCatchExceptions(mControl, wxEVT_COLOURPICKER_CHANGED, &TransitionParameterColor::onColor, this);
    return mControl;
}
void TransitionParameterColor::destroyWidget() 
{
    ASSERT_DIFFERS(mControl, 0);
    mControl->Unbind(wxEVT_COLOURPICKER_CHANGED, &TransitionParameterColor::onColor, this);
    mControl->Destroy();
    mControl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxColour TransitionParameterColor::getColor() const
{
    return mColor;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterColor::onColor(wxColourPickerEvent& event)
{
    if (mColor != event.GetColour())
    {
        signalUpdate([this, event] { mColor = event.GetColour(); });
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterColor& obj)
{
    os << obj.mColor;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterColor::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TransitionParameter);
        ar & BOOST_SERIALIZATION_NVP(mColor);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void TransitionParameterColor::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterColor::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterColor)
