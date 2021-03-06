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

#include "TransitionParameter.h"

#include "UtilWindow.h"

namespace model {

DEFINE_EVENT(EVENT_TRANSITION_PARAMETER_CHANGED, EventTransitionParameterChanged, wxString);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameter::TransitionParameter(const TransitionParameter& other)
    : wxEvtHandler()
    , mName{ other.mName }
    , mDescription{ other.mDescription }
    , mToolTip{ other.mToolTip }
    , mBitmap{ other.mBitmap }
{
}

//////////////////////////////////////////////////////////////////////////
// INTERFACE
//////////////////////////////////////////////////////////////////////////

wxString TransitionParameter::getName() const
{
    return mName;
}

void TransitionParameter::setName(const wxString& name)
{
    mName = name;
}

wxString TransitionParameter::getDescription() const
{
    return mDescription;
}

void TransitionParameter::setDescription(const wxString& description)
{
    mDescription = description;
}

wxString TransitionParameter::getToolTip() const
{
    return mToolTip;
}

void TransitionParameter::setToolTip(const wxString& tooltip)
{
    mToolTip = tooltip;
}

wxBitmap TransitionParameter::getBitmap()
{
    return mBitmap;
}

void TransitionParameter::setBitmap(const wxString& name)
{
    mBitmap = util::window::getBitmap(name);
}

void TransitionParameter::setOnChanged(std::function<void(const wxString&)> onChange) { mOnChange = onChange; }

//////////////////////////////////////////////////////////////////////////
// TO BE CALLED WHEN THE DATA CHANGES
//////////////////////////////////////////////////////////////////////////

void TransitionParameter::signalUpdate()
{
    EventTransitionParameterChanged changedEvent{ getDescription() };
    ProcessEvent(changedEvent);
    if (mOnChange != nullptr)
    {
        mOnChange(mName);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameter& obj)
{
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameter::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        // NOT: mDescription; -- should never be persisted.
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void TransitionParameter::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameter::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameter)
