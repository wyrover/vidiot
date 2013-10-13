// Copyright 2013 Eric Raijmakers.
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

#include "TransitionFactory.h"

#include "Transition.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionFactory::TransitionFactory(std::string type)
    :   mType(type)
    ,   mDefault()
{
}

TransitionFactory::~TransitionFactory()
{
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONS
//////////////////////////////////////////////////////////////////////////

TransitionDescriptions TransitionFactory::getAllPossibleTransitions() const
{
    TransitionDescriptions result;
    for ( auto kvp : mTransitions )
    {
        result.push_back(kvp.first);
    }
    return result;
}

TransitionPtr TransitionFactory::getDefault()
{
    ASSERT_NONZERO(mDefault);
    return make_cloned<Transition>(mDefault);
}

TransitionPtr TransitionFactory::getTransition(TransitionDescription description) const
{
    ASSERT_MAP_CONTAINS(mTransitions,description);
    return make_cloned<Transition>(mTransitions.find(description)->second);
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TransitionFactory::add(TransitionDescription description, TransitionPtr transition)
{
    ASSERT_MAP_CONTAINS_NOT(mTransitions,description);
    mTransitions[description] = transition;
}

void TransitionFactory::setDefault(TransitionPtr transition)
{
    mDefault = transition;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TransitionFactory& obj )
{
    os << obj.mType << '|' << obj.mDefault << '|' << obj.mTransitions;
    return os;
}

} //namespace