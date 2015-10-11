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

#pragma once

#include "AClipEdit.h"
#include "PositionInfo.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace command {

class EditClipSpeed
    :   public AClipEdit
{
public:

    static const rational sMinimumSpeedAllowed;
    static const rational sMaximumSpeedAllowed;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \param sequence sequence that is adjusted
    /// \param clip clip to be changed
    EditClipSpeed(
        const model::SequencePtr& sequence, 
        const model::IClipPtr& clip,
        const model::IClipPtr& link,
        const model::IClipPtr& clipClone,
        const model::IClipPtr& linkClone,
        rational speed);

    virtual ~EditClipSpeed();

    //////////////////////////////////////////////////////////////////////////
    // ROOTCOMMAND
    //////////////////////////////////////////////////////////////////////////

    bool isPossible() override;

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;
    void doExtraAfter() override;
    void undoExtraAfter() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getClip() const;
    rational getActualSpeed() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip;
    model::IClipPtr mClipClone;
    model::IClipPtr mLink;
    model::IClipPtr mLinkClone;
    boost::rational<int> mSpeed;
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
