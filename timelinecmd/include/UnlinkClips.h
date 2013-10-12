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

#ifndef UNLINK_CLIPS_H
#define UNLINK_CLIPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class UnlinkClips
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit UnlinkClips(model::SequencePtr sequence, model::IClips clips);

    virtual ~UnlinkClips();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const UnlinkClips& obj );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClips mClips; ///< List of clips to be unlinked
};

}}} // namespace

#endif // UNLINK_CLIPS_H