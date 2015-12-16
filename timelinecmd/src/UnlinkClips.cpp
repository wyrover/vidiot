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

#include "UnlinkClips.h"

#include "IClip.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilClone.h"

namespace gui { namespace timeline { namespace cmd {

UnlinkClips::UnlinkClips(const model::SequencePtr& sequence, const model::IClips& clips)
    :   AClipEdit(sequence)
    ,   mClips(clips)
{
    VAR_INFO(this)(mClips);
    mCommandName = _("Unlink clips");
}

UnlinkClips::~UnlinkClips()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void UnlinkClips::initialize()
{
    for ( model::IClipPtr clip : mClips )
    {
        // Replace clip with unlinked close.
        // The 'false' ensures that the link is not restored in 'AClipEdit::replaceLinks'
        replaceClip(clip, { make_cloned<model::IClip>(clip) }, false);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const UnlinkClips& obj)
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mClips;
    return os;
}

}}} // namespace