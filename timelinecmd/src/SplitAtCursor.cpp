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

#include "SplitAtCursor.h"

#include "Timeline.h"
#include "Cursor.h"
#include "Track.h"
#include "Selection.h"
#include "Sequence.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

SplitAtCursor::SplitAtCursor(const model::SequencePtr& sequence)
    :   AClipEdit(sequence)
{
    VAR_INFO(this);
    mCommandName = _("Split at cursor");
}

SplitAtCursor::~SplitAtCursor()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void SplitAtCursor::initialize()
{
    VAR_INFO(this);
    pts position = getTimeline().getCursor().getLogicalPosition();
    splittrack(getTimeline().getSequence()->getVideoTracks(), position);
    splittrack(getTimeline().getSequence()->getAudioTracks(), position);
}

void SplitAtCursor::doExtraBefore()
{
    storeSelection();
}

void SplitAtCursor::doExtraAfter()
{
    // Select clips just before the split
    model::IClips clipsBeforeSplit;
    for (model::TrackPtr track : getSequence()->getTracks())
    {
        model::IClipPtr clip{ track->getClip(getTimeline().getCursor().getLogicalPosition()) };
        if (clip &&
            clip->getPrev() &&
            !clip->getPrev()->isA<model::EmptyClip>())
        {
            clipsBeforeSplit.push_back(clip->getPrev());
        }
    }
    getTimeline().getSelection().change(clipsBeforeSplit);
}

void SplitAtCursor::undoExtraAfter()
{
    restoreSelection();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void SplitAtCursor::splittrack(model::Tracks tracks, pts position)
{
    for ( model::TrackPtr track : tracks )
    {
        split(track, position);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const SplitAtCursor& obj)
{
    os << static_cast<const AClipEdit&>(obj);
    return os;
}

}}} // namespace