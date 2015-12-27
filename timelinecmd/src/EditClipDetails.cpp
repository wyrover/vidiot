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

#include "Convert.h"
#include "IClip.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"

namespace gui { namespace timeline { namespace cmd {

EditClipDetails::EditClipDetails(
    const model::SequencePtr& sequence,
        const wxString& message,
    const model::IClipPtr& clip,
    const model::IClipPtr& clipClone)
    : AClipEdit(sequence)
    , mMessage(message)
    , mClip(clip)
    , mClipClone(clipClone)
    , mLink(clip->getLink())
    , mLinkClone(clipClone->getLink())
{
    VAR_INFO(this)(mClip)(mLink);
    mCommandName = wxString::Format(message, mClip->getDescription());
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
        ASSERT_NONZERO(mLinkClone);
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

wxString EditClipDetails::getMessage() const
{
    return mMessage;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const EditClipDetails& obj)
{
    os  << static_cast<const AClipEdit&>(obj) << '|' 
        << obj.mMessage << '|'
        << obj.mClip << '|' 
        << obj.mClipClone << '|'
        << obj.mLink << '|'
        << obj.mLinkClone;
    return os;
}

}}} // namespace