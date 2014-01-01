// Copyright 2013,2014 Eric Raijmakers.
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

#include "Cursor.h"
#include "Logging.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimelinesView.h"
#include "Track.h"
#include "TrimClip.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {

struct MoveCursor
    :   public AClipEdit
{
    MoveCursor(model::SequencePtr sequence, pts position)
        :   AClipEdit(sequence)
        ,   mOldPosition(getTimeline().getCursor().getLogicalPosition())
        ,   mNewPosition(position)
    {}

    void doExtraBefore()
    {
        getTimeline().getCursor().setLogicalPosition(mNewPosition);
    }

    void undoExtraAfter()
    {
        getTimeline().getCursor().setLogicalPosition(mOldPosition);
    }

private:
    pts mOldPosition;
    pts mNewPosition;
};

struct BeginTransaction
    :   public AClipEdit
{
    BeginTransaction(model::SequencePtr sequence)
        :   AClipEdit(sequence)
    {}

    void doExtraBefore()
    {
        getTimeline().beginTransaction();
    }

    void undoExtraAfter()
    {
        getTimeline().endTransaction();
    }
};

struct EndTransaction
    :   public AClipEdit
{
    EndTransaction(model::SequencePtr sequence)
        :   AClipEdit(sequence)
    {}

    void doExtraBefore()
    {
        getTimeline().endTransaction();
    }

    void undoExtraAfter()
    {
        getTimeline().beginTransaction();
    }
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

SplitAtCursorAndTrim::SplitAtCursorAndTrim(model::SequencePtr sequence, bool backwards)
    :   Combiner()
    ,   mSequence(sequence)
    ,   mBackwards(backwards)
    ,   mPosition(gui::TimelinesView::get().getTimeline(sequence).getCursor().getLogicalPosition())
    ,   mPossible(false)
{
    VAR_INFO(this)(mPosition)(mBackwards);

    int nVideo = 0;
    int nAudio = 0;
    int nTransition = 0;
    model::IClipPtr videoClip;
    model::IClipPtr audioClip;

    //////////////////////////////////////////////////////////////////////////

    std::set<pts> cuts = mSequence->getCuts();
    if (cuts.find(mPosition) != cuts.end())
    {
        gui::StatusBar::get().timedInfoText(_("Can't trim: cut at cursor position."));
    }
    else
    {
        for ( model::TrackPtr track : mSequence->getTracks() )
        {
            model::IClipPtr clip = mBackwards ? track->getClip(mPosition - 1) : track->getClip(mPosition);
            if (clip)
            {
                if (clip->isA<model::VideoClip>())
                {
                    videoClip = clip;
                    nVideo++;
                }
                else if (clip->isA<model::AudioClip>())
                {
                    audioClip = clip;
                    nAudio++;
                }
                else if (clip->isA<model::Transition>())
                {
                    nTransition++;
                }
            }
        }
        if (nVideo == 0 && nAudio == 0)
        {
            gui::StatusBar::get().timedInfoText(_("Nothing to be trimmed."));
        }
        else if (nTransition > 0)
        {
            gui::StatusBar::get().timedInfoText(_("Can't trim: transition."));
        }
        else if (nVideo > 1 || nAudio > 1)
        {
            gui::StatusBar::get().timedInfoText(_("Can't trim: too many clips at cursor position."));
        }
        else
        {
            model::IClipPtr clip = videoClip ? videoClip : audioClip;
            ASSERT_NONZERO(clip);
            command::TrimClip* cmd = new command::TrimClip(sequence, clip, model::TransitionPtr(), mBackwards ? ClipBegin : ClipEnd);

            pts left = clip->getLeftPts(); // Clip will be changed by the trim below, thus store here
            pts right = clip->getRightPts(); // Clip will be changed by the trim below, thus store here

            pts trim = mBackwards ? mPosition - left : mPosition - right;
            cmd->update(trim,true);
            if (cmd->getDiff() != 0)
            {
                mPossible = true;
                add(new BeginTransaction(sequence));
                add(cmd);
                if (mBackwards)
                {
                    add(new MoveCursor(sequence, left));
                }
                add(new EndTransaction(sequence)); // Added to avoid temporarily showing the wrong frame between the Trim::update() and the submission of the MoveCursor command.
            }
            else
            {
                gui::StatusBar::get().timedInfoText(_("Can't trim: position of clips prevents proper shifting"));
                delete cmd;
            }
        }
    }

    setName( _("Split at cursor position and remove ") + (backwards ? _("begin") : _("end")) + " of clip");
}

SplitAtCursorAndTrim::~SplitAtCursorAndTrim()
{
}

bool SplitAtCursorAndTrim::isPossible()
{
    return mPossible;
}

}}} // namespace