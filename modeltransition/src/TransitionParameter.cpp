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

#include "TransitionParameter.h"

#include "UtilLog.h"

namespace model {

DEFINE_EVENT(EVENT_TRANSITION_PARAMETER_CHANGING, EventTransitionParameterChanging, void*);
DEFINE_EVENT(EVENT_TRANSITION_PARAMETER_CHANGED, EventTransitionParameterChanged, void*);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameter::TransitionParameter()
{
    VAR_DEBUG(*this);
}

TransitionParameter::TransitionParameter(const TransitionParameter& other)
{
    VAR_DEBUG(*this)(other);
}

TransitionParameter::~TransitionParameter()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TO BE CALLED WHEN THE DATA CHANGES
//////////////////////////////////////////////////////////////////////////

void TransitionParameter::signalUpdate(std::function<void()> update)
{
    EventTransitionParameterChanging changingEvent{ nullptr };
    ProcessEvent(changingEvent);
    update();
    EventTransitionParameterChanged changedEvent{ nullptr };
    ProcessEvent(changedEvent);
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
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void TransitionParameter::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameter::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameter)
