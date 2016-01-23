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

namespace gui { namespace timeline { namespace cmd {

class EditClipDetails
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Make clones for the given clip and it's link. 
    /// Note that no actual change will be made to the timeline.
    /// That must be done upon the first actual change to the clone(s).
    /// \param sequence sequence in which the replacement is done.
    /// \param clip clip that was selected in the details view.
    explicit EditClipDetails(
        const model::SequencePtr& sequence, 
        const wxString& message,
        const model::IClipPtr& clip,
        const model::IClipPtr& clipClone);

    virtual ~EditClipDetails();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    void doExtraAfter() override;
    void undoExtraAfter() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxString getMessage() const;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const EditClipDetails& obj);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip;
    model::IClipPtr mClipClone;
    model::IClipPtr mLink; // Must be kept as member to avoid ref count reaching 0
    model::IClipPtr mLinkClone; // Must be kept as member to avoid ref count reaching 0
};

}}} // namespace
