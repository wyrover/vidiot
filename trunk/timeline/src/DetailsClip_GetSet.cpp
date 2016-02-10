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

#include "AudioClip.h"
#include "AudioKeyFrame.h"
#include "Convert.h"
#include "Transition.h"
#include "TransitionParameter.h"
#include "Config.h"
#include "VideoClip.h"
#include "VideoKeyFrame.h"
#include "VideoTrack.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr DetailsClip::getClip() const
{
    return mClip;
}

void DetailsClip::setClip(const model::IClipPtr& clip)
{
    VAR_DEBUG(clip);

    // Avoid useless updating. The position check ensures updating after moving a clip in the timeline (DND).
    if (mClip == clip && (!mClip || mClip->getLeftPts() == mClipPosition)) return;

    destroyTransitionParameterWidgets();

    mClip = nullptr;
    mClipPosition = 0;
    mTransitionClone = nullptr;
    mEditCommand = nullptr;
    mEditSpeedCommand = nullptr;

    model::VideoClipPtr video{ getClipOfType<model::VideoClip>(clip) };
    model::AudioClipPtr audio{ getClipOfType<model::AudioClip>(clip) };
    model::TransitionPtr transition{ getClipOfType<model::Transition>(clip) };

    bool autoStartPlayback{ Config::get().read<bool>(Config::sPathEditAutoStartPlayback) };
    bool play{ false };
    if (transition != nullptr && transition->isA<model::IVideo>())
    {
        play = autoStartPlayback;

        if (!play)
        {
            // Check if playback active, and the clip with the same index is still selected.
            // Continue the playback in that case.
            // Example: Change transition type, then undo the change. After the undo, playback must remain active.
            if (mPlaybackActive)
            {
                std::pair<int, int> index{ std::make_pair(clip->getIndex(), clip->getTrack()->getIndex()) };
                if (mPlaybackClipIndex == index)
                {
                    play = true;
                }
            }
        }
    }


    if (video != nullptr ||
        audio != nullptr ||
        transition != nullptr)
    {
        // Clip type for which editing details is supported.
        mClip = clip;
        mClipPosition = mClip->getLeftPts();

        // Must be done before adding/showing/hiding controls, because the computation for key frames buttons (which buttons to show)
        // depend on calculating the required widget size. Required widget size is not updated if the entire panel is hidden.
        requestShowAndUpdateTitle();

        // Length/speed
        // For audio/video clips and transitions, the length can be edited.
        updateLengthButtons();

        mSpeedSlider->Enable(video && !audio);
        mSpeedSpin->Enable(video && !audio);

        if (video && !audio)
        {
            if (!audio)
            {
                rational64 speed = boost::dynamic_pointer_cast<model::ClipInterval>(mClip)->getSpeed();
                mSpeedSlider->SetValue(factorToSliderValue(speed));
                mSpeedSpin->SetValue(boost::rational_cast<double>(speed));
            }
        }

        if (transition)
        {
            makeTransitionCloneAndCreateTransitionParameterWidgets(mClip);
        }

        mLastEditKeyFrameOffset = -1;
    }
    else
    {
        requestShow(false);
    }

    // Note: disabling a control and then enabling it again can cause extra events (value changed).
    // Therefore this has been placed here, to only dis/enable in the minimal number of cases.
    showOption(mLengthPanel, video != nullptr || audio  != nullptr || transition != nullptr);
    showOption(mPlaybackPanel, transition != nullptr && transition->isA<model::IVideo>());
    showOption(mTransitionTypePanel, transition != nullptr && transition->isA<model::IVideo>());
    mTransitionPanel->Show(transition != nullptr && transition->isA<model::IVideo>());
    showOption(mSpeedPanel, video  != nullptr|| audio != nullptr);
    mVideoKeyFrameControls->update();
    mAudioKeyFrameControls->update();

    Layout();

    mAutoPlayButton->SetValue(autoStartPlayback);
    startPlayback(play);
}

pts DetailsClip::getLength(wxToggleButton* button) const
{
    ASSERT_LESS_THAN(button->GetId(), static_cast<int>(mLengths.size()));
    return model::Convert::timeToPts(mLengths[button->GetId()]);
}

}} // namespace
