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

#ifndef CREATE_TRANSITION_H
#define CREATE_TRANSITION_H

#include "AClipEdit.h"
#include "Enums.h"

namespace gui { namespace timeline { namespace command {

class CreateTransition
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CreateTransition(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, model::TransitionType type);

    virtual ~CreateTransition();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Determine if a transition is possible
    bool isPossible();

    model::IClipPtr getLeftClip() const;
    model::IClipPtr getRightClip() const;

    pts getLeftSize() const;
    pts getRightSize() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::TransitionPtr mTransition;

    model::TransitionType mType;

    model::IClipPtr mLeft;
    model::IClipPtr mRight;

    pts mLeftSize;
    pts mRightSize;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const CreateTransition& obj );
};

}}} // namespace

#endif // CREATE_TRANSITION_H