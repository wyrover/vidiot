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

#include "CreateTransitionHelper.h"

#include "Combiner.h"
#include "Config.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "Mouse.h"
#include "ProjectModification.h"
#include "StatusBar.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "TrimClip.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoTransition.h"

namespace gui { namespace timeline { namespace command {

void createTransition(const model::SequencePtr& sequence, const model::IClipPtr& clip, const model::TransitionType& type, const model::TransitionPtr& transition)
{
    VAR_INFO(type);
    ASSERT(sequence);
    ASSERT(clip);
    ASSERT(!clip->isA<model::EmptyClip>());
    ASSERT(transition);

    Timeline& timeline = gui::TimelinesView::get().getTimeline(sequence);

    timeline.beginTransaction();
    command::CreateTransition* createTransitionCommand = new command::CreateTransition(sequence, clip, transition, type);
    model::IClipPtr leftClip = createTransitionCommand->getLeftClip();
    model::IClipPtr rightClip = createTransitionCommand->getRightClip();
    if (!model::ProjectModification::submitIfPossible(createTransitionCommand))
    {
        if (leftClip && rightClip &&
            ((type == model::TransitionTypeInOut || type == model::TransitionTypeOutIn)))
        {
            pts defaultSize = Config::ReadLong(Config::sPathDefaultTransitionLength);

            // Ensure that the transition can be made by shortening the clips, if required (and, if possible)
            model::TrackPtr track = clip->getTrack();
            model::IClipPtr prevClip = leftClip->getPrev(); // Temporarily stored to retrieve the (new) trimmed clips again. NOTE: This may be 0 if leftClip is the first clip of the track!!!

            command::TrimClip* trimLeftCommand = 0;
            command::TrimClip* trimRightCommand = 0;

            pts extraNeededLeft = leftClip->getMaxAdjustEnd() - (defaultSize / 2);
            if (extraNeededLeft < 0)
            {
                trimLeftCommand = new command::TrimClip(sequence, leftClip, model::TransitionPtr(), ClipEnd);
                trimLeftCommand->update(extraNeededLeft, true);
            }

            leftClip = prevClip ? prevClip->getNext() : track->getClips().front(); // Left clip is changed by the trim left command
            model::IClipPtr rightClip = leftClip->getNext();

            pts extraNeededRight = rightClip->getMinAdjustBegin() + (defaultSize / 2);
            if (extraNeededRight > 0)
            {
                trimRightCommand = new command::TrimClip(sequence, rightClip, model::TransitionPtr(), ClipBegin);
                trimRightCommand->update(extraNeededRight, true);
            }

            command::CreateTransition* createTransitionCommand = new command::CreateTransition(sequence, leftClip, transition, model::TransitionTypeOutIn);

            if (createTransitionCommand->isPossible())
            {
                ::command::Combiner* combiner = new ::command::Combiner();
                if (trimLeftCommand)
                {
                    combiner->add(trimLeftCommand);
                }
                if (trimRightCommand)
                {
                    combiner->add(trimRightCommand);
                }
                combiner->add(createTransitionCommand);
                combiner->submit();

                trimLeftCommand = 0; // Ownership transferred to Combiner
                trimRightCommand = 0; // Ownership transferred to Combiner
                createTransitionCommand = 0; // Ownership transferred to Combiner
            }
            delete trimLeftCommand; // Triggers a revert also
            delete trimRightCommand; // Triggers a revert also
            if (createTransitionCommand) // If the command was not submitted, then that was because 'isPossible()' never returned true.
            {
                gui::StatusBar::get().timedInfoText(_("Unable to make room for adding the transition."));
            }
            delete createTransitionCommand;
        }
    }
    timeline.endTransaction();
}

}}} // namespace