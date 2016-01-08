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
#include "VideoClip.h"
#include "VideoKeyFrame.h"

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
    mTransitionClone = nullptr;
    mEditCommand = nullptr;
    mEditSpeedCommand = nullptr;

    model::VideoClipPtr video{ getClipOfType<model::VideoClip>(clip) };
    model::AudioClipPtr audio{ getClipOfType<model::AudioClip>(clip) };
    model::TransitionPtr transition{ getClipOfType<model::Transition>(clip) };

    if (video != nullptr ||
        audio != nullptr ||
        transition != nullptr)
    {
        // Clip type for which editing details is supported.
        mClip = clip;
        mClipPosition = mClip->getLeftPts();

        // Must be done before adding/showing/hiding controls, because the computation for key frames buttons (which buttons to show)
        // depend on calculating the required widget size. Required widget size is not updated if the entire panel is hidden.
        requestShow(true, mClip->getDescription() + " (" + model::Convert::ptsToHumanReadibleString(mClip->getPerceivedLength()) + "s)");

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
        mLastEditKeyFrameOffset = -1;
    }
    else
    {
        requestShow(false);
    }

    // Note: disabling a control and then enabling it again can cause extra events (value changed).
    // Therefore this has been placed here, to only dis/enable in the minimal number of cases.
    showOption(mLengthPanel, video != nullptr || audio  != nullptr || transition != nullptr);
    showOption(mSpeedPanel, video  != nullptr|| audio != nullptr);
    mVideoKeyFrameControls->update();
    mAudioKeyFrameControls->update();

    Layout();
}

pts DetailsClip::getLength(wxToggleButton* button) const
{
    ASSERT_LESS_THAN(button->GetId(), static_cast<int>(mLengths.size()));
    return model::Convert::timeToPts(mLengths[button->GetId()]);
}

}} // namespace
