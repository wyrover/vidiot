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

#ifndef DELETE_SELECTED_CLIPS_H
#define DELETE_SELECTED_CLIPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class DeleteSelectedClips
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DeleteSelectedClips(model::SequencePtr sequence);

    ~DeleteSelectedClips();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;
    void doExtraBefore() override;
    void undoExtraAfter() override;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const DeleteSelectedClips& obj );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mShift;             ///< True if the empty area left over after deletion should be trimmed
    model::IClips mSelected; ///< List of clips that was selected before the edit operation was done

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Store the list of currently selected clips
    void storeSelection();

    /// Restore the list of selected clips as was saved before applying the edit
    void restoreSelection();

};

}}} // namespace

#endif // DELETE_SELECTED_CLIPS_H