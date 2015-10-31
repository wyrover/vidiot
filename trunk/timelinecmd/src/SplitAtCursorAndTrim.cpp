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

#include "SplitAtCursorAndTrim.h"

#include "Cursor.h"
#include "Logging.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimelinesView.h"
#include "Track.h"
#include "TrimClip.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {


//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void SplitAtCursorAndTrim(const model::SequencePtr& sequence, bool backwards)
{

    Timeline& timeline{ gui::TimelinesView::get().getTimeline(sequence) };
    pts position{ timeline.getCursor().getLogicalPosition() };

    VAR_INFO(position)(backwards);

    int nVideo = 0;
    int nAudio = 0;
    int nTransition = 0;
    model::IClipPtr videoClip;
    model::IClipPtr audioClip;

    //////////////////////////////////////////////////////////////////////////

    std::set<pts> cuts = sequence->getCuts();
    if (cuts.find(position) != cuts.end())
    {
        gui::StatusBar::get().timedInfoText(_("Can't trim: cut at cursor position."));
    }
    else
    {
        for ( model::TrackPtr track : sequence->getTracks() )
        {
            model::IClipPtr clip = backwards ? track->getClip(position - 1) : track->getClip(position);
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
            timeline.getSelection().updateOnTrim(clip);
            command::TrimClip* cmd = new command::TrimClip(sequence, clip, model::TransitionPtr(), backwards ? ClipBegin : ClipEnd);

            pts left = clip->getLeftPts(); // Clip will be changed by the trim below, thus store here
            pts right = clip->getRightPts(); // Clip will be changed by the trim below, thus store here

            pts trim = backwards ? position - left : position - right;
            cmd->update(trim,true,true);
            cmd->setCursorPositionAfter(backwards ? left : position);
            if (cmd->getDiff() != 0)
            {
                cmd->submit();
            }
            else
            {
                gui::StatusBar::get().timedInfoText(_("Can't trim: position of clips prevents proper shifting"));
                delete cmd;
            }
        }
    }
}

}}} // namespace
