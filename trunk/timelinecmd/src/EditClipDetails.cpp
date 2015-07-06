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

#include "EditClipDetails.h"

#include "IClip.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilClone.h"

namespace gui { namespace timeline { namespace command {

EditClipDetails::EditClipDetails(const model::SequencePtr& sequence, const model::IClipPtr& clip)
    : AClipEdit(sequence)
    , mClip(clip)
    , mClipClone(make_cloned<model::IClip>(clip))
    , mLink(mClip->getLink())
    , mLinkClone(mLink ? make_cloned<model::IClip>(mLink) : nullptr)
{
    VAR_INFO(this)(mClip)(mLink);
    mCommandName = _("Edit " + mClip->getDescription());
}

EditClipDetails::~EditClipDetails()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void EditClipDetails::initialize()
{
    replaceClip(mClip, { mClipClone });
    if (mLink != nullptr)
    {
        replaceClip(mLink, { mLinkClone }); 
    }
}

void EditClipDetails::doExtraAfter()
{
    // Ensure that this clip is selected again after Undo/Redo.
    // Otherwise, triggering Undo/Redo may cause more than one clip being
    // selected, which looks rather odd.
    getTimeline().getSelection().change({ mClipClone, mLinkClone });
}

void EditClipDetails::undoExtraAfter()
{
    // Ensure that this clip is selected again after Undo/Redo.
    // Otherwise, triggering Undo/Redo may cause more than one clip being
    // selected, which looks rather odd.
    getTimeline().getSelection().change({ mClip, mLink});
}


//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr EditClipDetails::getClipClone() const
{
    return mClipClone;
}

model::IClipPtr EditClipDetails::getLinkClone() const
{
    return mLinkClone;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const EditClipDetails& obj)
{
    os  << static_cast<const AClipEdit&>(obj) << '|' 
        << obj.mClip << '|' 
        << obj.mClipClone << '|'
        << obj.mLink << '|'
        << obj.mLinkClone;
    return os;
}

}}} // namespace