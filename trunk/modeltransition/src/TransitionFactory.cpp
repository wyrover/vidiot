// Copyright 2013-2016 Eric Raijmakers.
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

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionFactory::TransitionFactory(const std::string& type)
    :   mType(type)
{
}

TransitionFactory::~TransitionFactory()
{
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONS
//////////////////////////////////////////////////////////////////////////

std::vector<model::TransitionPtr> TransitionFactory::getAllPossibleTransitions() const
{
    std::vector<model::TransitionPtr> result;
    for ( auto t : mTransitions )
    {
        result.push_back(make_cloned<model::Transition>(t));
    }
    return result;
}

TransitionPtr TransitionFactory::getDefault()
{
    ASSERT_NONZERO(mTransitions.size());
    return make_cloned<Transition>(mTransitions.front());
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TransitionFactory::add(const TransitionPtr& transition)
{
    mTransitions.push_back(transition);
    transition->initParameters();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionFactory& obj)
{
    os << obj.mType << '|' << obj.mTransitions;
    return os;
}

} //namespace