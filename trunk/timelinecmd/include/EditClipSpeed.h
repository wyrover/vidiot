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

#pragma once

#include "AClipEdit.h"
#include "PositionInfo.h"

namespace gui { namespace timeline { namespace cmd {

class EditClipSpeed
    :   public AClipEdit
{
public:

    static const rational64 sMinimumSpeedAllowed;
    static const rational64 sMaximumSpeedAllowed;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \param sequence sequence that is adjusted
    /// \param clip clip to be changed
    EditClipSpeed(
        const model::SequencePtr& sequence, 
        const model::IClipPtr& clip,
        const model::IClipPtr& clipClone,
        rational64 speed);

    virtual ~EditClipSpeed();

    //////////////////////////////////////////////////////////////////////////
    // ROOTCOMMAND
    //////////////////////////////////////////////////////////////////////////

    bool isPossible() const override;

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getClip() const;
    rational64 getActualSpeed() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip;
    model::IClipPtr mClipClone;
    model::IClipPtr mLink; // Must be kept as member to avoid ref count reaching 0
    model::IClipPtr mLinkClone; // Must be kept as member to avoid ref count reaching 0
    rational64 mSpeed;
    bool mPossible;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Determine the maximum allowed speed, with the given clip's offset.
    /// This takes into account room required for any adjacent transitions.
    /// \param clipInterval clip to be checked.
    void adjustSpeedForClipBounds(model::IClipPtr clip);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const EditClipSpeed& obj);
};

}}} // namespace
