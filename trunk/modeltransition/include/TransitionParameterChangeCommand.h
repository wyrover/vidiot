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

#ifndef TRANSITION_PARAMETER_CHANGE_COMMAND_H
#define TRANSITION_PARAMETER_CHANGE_COMMAND_H

#include "RootCommand.h"

namespace model {

class TransitionParameterChangeCommand
    :   public ::command::RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit TransitionParameterChangeCommand(const TransitionPtr& transition);
    virtual ~TransitionParameterChangeCommand();

    void setNewParameters();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::AudioClipPtr getAudioClip() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;

    model::TransitionPtr mTransition;
    std::map<int, TransitionParameterPtr> mOld;
    boost::optional< std::map<int, TransitionParameterPtr> > mNew;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterChangeCommand& obj);
};

} // namespace

#endif
