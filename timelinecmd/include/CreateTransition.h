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

#pragma once

#include "AClipEdit.h"
#include "Enums.h"

namespace gui { namespace timeline { namespace cmd {

class CreateTransition
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CreateTransition(const model::SequencePtr& sequence, const model::IClipPtr& clip, const model::TransitionPtr& transition, const model::TransitionType& type);

    virtual ~CreateTransition();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool isPossible() const override;

    model::IClipPtr getLeftClip() const;
    model::IClipPtr getRightClip() const;

    boost::optional<pts> getLeftSize() const;
    boost::optional<pts> getRightSize() const;

    pts getLength() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::TransitionPtr mTransition;

    model::TransitionType mType;

    model::IClipPtr mLeft;
    model::IClipPtr mRight;

    boost::optional<pts> mLeftSize;
    boost::optional<pts> mRightSize;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const CreateTransition& obj);
};

}}} // namespace
