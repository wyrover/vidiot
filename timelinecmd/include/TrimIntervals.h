// Copyright 2013-2015 Eric Raijmakers.
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
#include "UtilInt.h"

namespace gui { namespace timeline {

namespace cmd {

class TrimIntervals
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Remove marked intervals from timeline
    /// \param sequence sequence from which regions must be removed
    /// \param remove list of intervals to be removed
    /// \param name name of command (for undo/redo menu entries)
    TrimIntervals(const model::SequencePtr& sequence, const PtsIntervals& remove, const wxString& name);

    virtual ~TrimIntervals();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    void doExtraBefore() override;
    void undoExtraAfter() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PtsIntervals mIntervals;    ///< Intervals as marked in the sequence before the command was created
    PtsIntervals mRemoved;      ///< Intervals to be removed from the sequence (maybe all 'non-marked' areas)

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TrimIntervals& obj);
};
}}} // namespace
