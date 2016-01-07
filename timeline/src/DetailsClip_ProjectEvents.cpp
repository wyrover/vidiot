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

#include "DetailsClip.h"

#include "VideoDisplayEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onTransitionParameterChanged(model::EventTransitionParameterChanged& event)
{
    submitEditCommandUponTransitionEdit(event.getValue());
    event.Skip();
}

void DetailsClip::onSelectionChanged(timeline::EventSelectionUpdate& event)
{
    VAR_DEBUG(this);
    CatchExceptions([this]
    {
        if (getTrim().isActive())
        {
            // At the end of the trim operation, the trim command is initialized.
            // In AClipEdit::initialize another EventSelectionUpdate is triggered.
            // Updating the current clip during trimming causes unnecessary
            // toggling (flickering between player - via DetailsClip::edit -
            // and the preview - via the trim preview).
            return;
        }
        std::set<model::IClipPtr> selection = getSequence()->getSelectedClips();
        model::IClipPtr selectedclip;
        VAR_DEBUG(selection.size());
        if (selection.size() == 1)
        {
            selectedclip = *selection.begin();
        }
        else if (selection.size() == 2)
        {
            model::IClipPtr a = *selection.begin();
            model::IClipPtr b = *(++(selection.begin()));
            if (a->getLink() == b)
            {
                ASSERT_EQUALS(b->getLink(), a);
                selectedclip = (a->isA<model::VideoClip>()) ? a : b; // Always use the videoclip (avoid problems in automated test as a result of timing differences - sometimes the videoclip is focused first, sometimes the audio clip)
            }
        }
        setClip(selectedclip);
    });
    event.Skip();
}

void DetailsClip::onPlaybackPosition(PlaybackPositionEvent& event)
{
    LOG_DEBUG;
    CatchExceptions([this]
    {
        mVideoKeyFrameControls->update();
        mAudioKeyFrameControls->update();
    });
    event.Skip();
}


}} // namespace
