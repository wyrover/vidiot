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

#include "DetailsClip.h"

#include "AudioClip.h"
#include "Convert.h"
#include "Transition.h"
#include "UtilException.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoClipKeyFrame.h"

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

    //todo this has become obsolete since no preview is shown but simply the current frame shown again in the player?
    //// For some edit operations the preview is shown iso player.
    //// The 'end' of the edit operation is not clear (edit opacity
    //// via the slider starts the operation, and the preview, but
    //// when does the operation end?).
    ////
    //// However, at some point the preview must be removed again.
    //// That's done here, when another (or no) clip is selected.
    //getTimeline().getPlayer()->showPlayer();

    if (mTransitionClone)
    {
        while (mTransitionBoxSizer->GetItemCount() > 0)
        {
            wxWindow* widget = mTransitionBoxSizer->GetItem(static_cast<size_t>(0))->GetWindow();
            mTransitionBoxSizer->Detach(0);
            if (dynamic_cast<wxStaticText*>(widget) != 0)
            {
                // It's a title. Must be removed also.
                widget->Destroy();
            }
        }
        for (auto id_and_parameter : mTransitionClone->getParameters())
        {
            id_and_parameter.second->Unbind(model::EVENT_TRANSITION_PARAMETER_CHANGED, &DetailsClip::onTransitionParameterChanged, this);
            id_and_parameter.second->destroyWidget();
        }
    }

    mClip = nullptr;
    mClipPosition = 0;
    mClones = nullptr;
    mTransitionClone = nullptr;
    mEditCommand = nullptr;
    mEditSpeedCommand = nullptr;
    updateProjectEventBindings(); // Unbind

    model::VideoClipPtr video{ getVideoClip(clip) };
    model::AudioClipPtr audio{ getAudioClip(clip) };
    model::TransitionPtr transition{ getTransition(clip) };

    if (video != nullptr ||
        audio != nullptr ||
        transition != nullptr)
    {
        // Clip type for which editing details is supported.
        mClip = clip;
        mClipPosition = mClip->getLeftPts();

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

        if (video)
        {
            updateVideoKeyFrameControls();
        }

        if (audio)
        {
            mVolumeSlider->SetValue(audio->getVolume());
            mVolumeSpin->SetValue(audio->getVolume());
        }

        if (transition)
        {
            mTransitionClone = make_cloned<model::Transition>(transition);
            setBox(mTransitionBoxSizer);
            for (auto id_and_parameter : mTransitionClone->getParameters())
            {
                addOption(
                    id_and_parameter.second->getName(),
                    id_and_parameter.second->makeWidget(this));
                id_and_parameter.second->Bind(model::EVENT_TRANSITION_PARAMETER_CHANGED, &DetailsClip::onTransitionParameterChanged, this);
            }
        }

        requestShow(true, mClip->getDescription() + " (" + model::Convert::ptsToHumanReadibleString(mClip->getPerceivedLength()) + "s)");
    }
    else
    {
        requestShow(false);
    }

    updateProjectEventBindings(); // Bind again
                                  
                                  
    // Note: disabling a control and then enabling it again can cause extra events (value changed).
    // Therefore this has been placed here, to only dis/enable in the minimal number of cases.
    showOption(mLengthPanel, video != nullptr || audio  != nullptr || transition != nullptr);
    showOption(mSpeedPanel, video  != nullptr|| audio != nullptr);
    showOption(mOpacityPanel, video != nullptr);
    showOption(mScalingPanel, video != nullptr);
    showOption(mRotationPanel, video != nullptr);
    showOption(mAlignmentPanel, video != nullptr);
    showOption(mVideoKeyFramesEditPanel, video != nullptr);
    showOption(mVolumePanel, audio != nullptr);

    Layout();
}

pts DetailsClip::getLength(wxToggleButton* button) const
{
    ASSERT_LESS_THAN(button->GetId(), static_cast<int>(mLengths.size()));
    return model::Convert::timeToPts(mLengths[button->GetId()]);
}

}} // namespace
