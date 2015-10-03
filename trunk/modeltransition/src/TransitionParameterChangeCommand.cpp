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

#include "TransitionParameterChangeCommand.h"

#include "Transition.h"
#include "TransitionParameter.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterChangeCommand::TransitionParameterChangeCommand(const TransitionPtr& transition) // todo remove this class. never used.
    :   RootCommand()
    ,   mInitialized(false)
    ,   mTransition(transition)
    ,   mOld(make_cloned<int,TransitionParameter>(transition->getParameters()))
    ,   mNew(boost::none)
{
    mCommandName = _("Change ") + transition->getDescription();
}

TransitionParameterChangeCommand::~TransitionParameterChangeCommand()
{
}

void TransitionParameterChangeCommand::setNewParameters()
{
    mNew.reset(make_cloned<int,TransitionParameter>(mTransition->getParameters()));
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool TransitionParameterChangeCommand::Do()
{
    VAR_INFO(*this)(mInitialized);

    if (mInitialized)
    {
        // Only the second time that Do() is called (redo) something actually needs to be done.
        // The first time is handled by the transition parameters themselves.
        if (mNew)
        {
            mTransition->setParameters(make_cloned<int,TransitionParameter>(*mNew));
        }
    }
    mInitialized = true;
    return true;
}

bool TransitionParameterChangeCommand::Undo()
{
    VAR_INFO(*this);
    if (mNew)
    {
        mTransition->setParameters(make_cloned<int,TransitionParameter>(mOld));
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterChangeCommand& obj)
{
    os  << &obj << '|'
        << typeid(obj).name()    << '|'
        << obj.mTransition       << '|'
        << obj.mOld              << '|'
        << obj.mNew;
    return os;
}

} // namespace