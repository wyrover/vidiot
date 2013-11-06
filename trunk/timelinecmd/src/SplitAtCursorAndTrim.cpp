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

#include "SplitAtCursorAndTrim.h"

#include "UtilLog.h"
#include "Timeline.h"
#include "Logging.h"
#include "Cursor.h"
#include "Track.h"
#include "Sequence.h"
#include "TimelinesView.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {

struct SelectClipsBesidesCursor
    :   public AClipEdit
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    SelectClipsBesidesCursor(model::SequencePtr sequence, bool backwards)
        :   AClipEdit(sequence)
        ,   mBackwards(backwards)
    {
    }

    virtual ~SelectClipsBesidesCursor()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override
    {
    }

    void doExtraBefore()
    {
        storeSelection();
        pts selectAtPosition = getTimeline().getCursor().getLogicalPosition();
        if (mBackwards)
        {
            selectAtPosition--; // Select left of the cut
        }
        model::IClips selected;
        for ( model::TrackPtr track : getTimeline().getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(selectAtPosition);
            if (clip)
            {
                if (clip->isA<model::VideoClip>() || clip->isA<model::AudioClip>())
                {
                    selected.push_back(clip);
                }
            }
        }
        getTimeline().getSelection().change(selected);
        getTimeline().getCursor().setLogicalPosition(selected.front()->getLeftPts());
    }

    void undoExtraAfter()
    {
        restoreSelection();
    }

    bool mBackwards;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

SplitAtCursorAndTrim::SplitAtCursorAndTrim(model::SequencePtr sequence, bool backwards)
    :   Combiner()
    ,   mSequence(sequence)
    ,   mBackwards(backwards)
    ,   mPosition(gui::TimelinesView::get().getTimeline(sequence).getCursor().getLogicalPosition())
{
    VAR_INFO(this)(mPosition)(mBackwards);
    // todo only if there's only one or two clips and they both are regular clips with the same begin point...
    add(new SplitAtCursor(sequence));
    add(new SelectClipsBesidesCursor(sequence, mBackwards));
    add(new DeleteSelectedClips(sequence, true));
    setName( _("Split at cursor position and remove beginning of clips") );
}

SplitAtCursorAndTrim::~SplitAtCursorAndTrim()
{
}

void SplitAtCursorAndTrim::submitIfPossible() // todo make generic mechanism for this (with a virtual isPossible())
{
    bool foundSplittableClip = false;
    for ( model::TrackPtr track : mSequence->getTracks() )
    {
        model::IClipPtr clipAtCursorPosition = track->getClip(mPosition);
        if ((clipAtCursorPosition->getLeftPts() == mPosition) ||
            (clipAtCursorPosition->getRightPts() == mPosition))
        {
            gui::StatusBar::get().timedInfoText(_("Can't trim: cut at cursor position."));
            return;
        }

        model::IClipPtr clipToBeSplit = mBackwards ? track->getClip(mPosition - 1) : clipAtCursorPosition;

        model::IClipPtr clip = mBackwards ? track->getClip(mPosition - 1) : track->getClip(mPosition);
        if (clip)
        {
            if (clip->isA<model::Transition>())
            {
                gui::StatusBar::get().timedInfoText(_("Can't trim: transition."));
                return;
            }
            else if (mBackwards && clip->getInTransition())
            {
                gui::StatusBar::get().timedInfoText(_("Can't trim: transition at begin."));
                return;
            }
            else if (!mBackwards && clip->getOutTransition())
            {
                gui::StatusBar::get().timedInfoText(_("Can't trim: transition at end."));
                return;
            }
            if (clip->isA<model::VideoClip>() || clip->isA<model::AudioClip>())
            {
                foundSplittableClip = true;
            }
        }
    }
    if (!foundSplittableClip)
    {
        gui::StatusBar::get().timedInfoText(_("Nothing to be trimmed."));
        return;
    }
    submit();
}

}}} // namespace