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
{
}

TransitionFactory::~TransitionFactory()
{
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONS
//////////////////////////////////////////////////////////////////////////

void TransitionFactory::add(TransitionPtr transition)
{
    mTransitions.push_back(transition);
}

TransitionPtr TransitionFactory::getDefault()
{
    ASSERT_NONZERO(mTransitions.size());
    return make_cloned<Transition>(mTransitions.front());
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TransitionFactory& obj )
{
    os << obj.mType << '|' << obj.mTransitions;
    return os;
}

} //namespace