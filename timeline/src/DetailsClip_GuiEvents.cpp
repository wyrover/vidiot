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

#include "Combiner.h"
#include "StatusBar.h"
#include "TrimClip.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onShow(wxShowEvent& event)
{
    //if (event.IsShown())
    //{
    //    updateLengthButtons();
    //}
    event.Skip();
}

void DetailsClip::onSize(wxSizeEvent& event)
{
    CatchExceptions([this]
    {
        mVideoKeyFrameControls->update();
        mAudioKeyFrameControls->update();
    });
    event.Skip();
}

void DetailsClip::onLengthButtonPressed(wxCommandEvent& event)
{
    wxToggleButton* button{ dynamic_cast<wxToggleButton*>(event.GetEventObject()) };
    CatchExceptions([this, button]
    {
        handleLengthButtonPressed(button);
    });
    event.Skip();
}

void DetailsClip::onSpeedSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mSpeedSlider->GetValue());
    CatchExceptions([this]
    {
        createOrUpdateSpeedCommand(sliderValueToFactor(mSpeedSlider->GetValue()));
    });
    event.Skip();
}

void DetailsClip::onSpeedSpinChanged(wxSpinDoubleEvent& event)
{
    double value{ mSpeedSpin->GetValue() }; // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    CatchExceptions([this, value]
    {
        int spinFactor{ static_cast<int>(floor(std::round(value * sFactorPrecisionFactor))) }; // This construct ensures that adding +0.01 is not ignored due to rounding issues
        rational64 s(spinFactor, sFactorPrecisionFactor);
        createOrUpdateSpeedCommand(s);
    });
    event.Skip();
}

void DetailsClip::onOpacitySliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mOpacitySlider->GetValue());
    VAR_ERROR(mOpacitySlider->GetValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditOpacity, true, [this]
        {
            mVideoKeyFrameControls->getKeyFrame()->setOpacity(mOpacitySlider->GetValue());
        });
    });
    event.Skip();
}

void DetailsClip::onOpacitySpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mOpacitySpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    CatchExceptions([this]
    {
        // Changes same clip aspect as the slider
        submitEditCommandUponAudioVideoEdit(sEditOpacity, true, [this]
        {
            mVideoKeyFrameControls->getKeyFrame()->setOpacity(mOpacitySpin->GetValue());
        });
    });
    event.Skip();
}

void DetailsClip::onScalingChoiceChanged(wxCommandEvent& event)
{
    VAR_INFO(mSelectScaling->getValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditScalingType, true, [this]
        {
            mVideoKeyFrameControls->getKeyFrame()->setScaling(mSelectScaling->getValue(), boost::none);
        });
    });
    event.Skip();
}

void DetailsClip::onScalingSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mScalingSlider->GetValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditScaling, true, [this]
        {
            mVideoKeyFrameControls->getKeyFrame()->setScaling(model::VideoScalingCustom, boost::optional< rational64 >(sliderValueToFactor(mScalingSlider->GetValue())));
        });
    });
    event.Skip();
}

void DetailsClip::onScalingSpinChanged(wxSpinDoubleEvent& event)
{
    double value{ mScalingSpin->GetValue() }; // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    VAR_INFO(value);
    CatchExceptions([this, value]
    {
        int spinFactor{ narrow_cast<int>(floor(std::round(value * sFactorPrecisionFactor))) }; // This construct ensures that adding +0.01 is not ignored due to rounding issues
        rational64 r(spinFactor, sFactorPrecisionFactor);
        // Changes same clip aspect as the slider
        submitEditCommandUponAudioVideoEdit(sEditScaling, true, [this, r]
        {
            mVideoKeyFrameControls->getKeyFrame()->setScaling(model::VideoScalingCustom, boost::optional< rational64 >(r));
        });
    });
    event.Skip();
}

void DetailsClip::onRotationSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mRotationSlider->GetValue());
    rational64 r(mRotationSlider->GetValue(), sRotationPrecisionFactor);
    CatchExceptions([this, r]
    {
        submitEditCommandUponAudioVideoEdit(sEditRotation, true, [this, r]
        {
            mVideoKeyFrameControls->getKeyFrame()->setRotation(r);
        });
    });
    event.Skip();
}

void DetailsClip::onRotationSpinChanged(wxSpinDoubleEvent& event)
{
    VAR_INFO(mRotationSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    int spinFactor{ narrow_cast<int>(floor(std::round(mRotationSpin->GetValue() * sRotationPrecisionFactor))) }; // This construct ensures that adding +0.01 is not ignored due to rounding issues
    rational64 r(spinFactor, sRotationPrecisionFactor);
    CatchExceptions([this,r]
    {
        // Changes same clip aspect as the slider
        submitEditCommandUponAudioVideoEdit(sEditRotation, true, [this, r]
        {
            mVideoKeyFrameControls->getKeyFrame()->setRotation(r);
        });
    });
    event.Skip();
}

void DetailsClip::onAlignmentChoiceChanged(wxCommandEvent& event)
{
    VAR_INFO(mSelectAlignment->getValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditAlignment, true, [this]
        {
            mVideoKeyFrameControls->getKeyFrame()->setAlignment(mSelectAlignment->getValue());
        });
    });
    event.Skip();
}

void DetailsClip::onPositionXSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionXSlider->GetValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditX, true, [this]
        {
            updateAlignment(true);
            mVideoKeyFrameControls->getKeyFrame()->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
        });
    });
    event.Skip();
}

void DetailsClip::onPositionXSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mPositionXSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    CatchExceptions([this]
    {
        // Changes same clip aspect as the slider
        submitEditCommandUponAudioVideoEdit(sEditX, true, [this]
        {
            updateAlignment(true);
            mVideoKeyFrameControls->getKeyFrame()->setPosition(wxPoint(mPositionXSpin->GetValue(), mPositionYSlider->GetValue()));
        });
    });
    event.Skip();
}

void DetailsClip::onPositionYSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mPositionYSlider->GetValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditY, true, [this]
        {
            updateAlignment(false);
            mVideoKeyFrameControls->getKeyFrame()->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSlider->GetValue()));
        });
    });
    event.Skip();
}

void DetailsClip::onPositionYSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mPositionYSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    CatchExceptions([this]
    {
        // Changes same clip aspect as the slider
        submitEditCommandUponAudioVideoEdit(sEditY, true, [this]
        {
            updateAlignment(false);
            mVideoKeyFrameControls->getKeyFrame()->setPosition(wxPoint(mPositionXSlider->GetValue(), mPositionYSpin->GetValue()));
        });
    });
    event.Skip();
}

void DetailsClip::onVolumeSliderChanged(wxCommandEvent& event)
{
    VAR_INFO(mVolumeSlider->GetValue());
    CatchExceptions([this]
    {
        submitEditCommandUponAudioVideoEdit(sEditVolume, false, [this]
        {
            mAudioKeyFrameControls->getKeyFrame()->setVolume(mVolumeSlider->GetValue());
        });
    });
    event.Skip();
}

void DetailsClip::onVolumeSpinChanged(wxSpinEvent& event)
{
    VAR_INFO(mVolumeSpin->GetValue()); // NOT: event.GetValue() -- The event's value may be outside the range boundaries
    CatchExceptions([this]
    {
        // Changes same clip aspect as the slider
        submitEditCommandUponAudioVideoEdit(sEditVolume, false, [this]
        {
            mAudioKeyFrameControls->getKeyFrame()->setVolume(mVolumeSpin->GetValue());
        });
    });
    event.Skip();
}

void DetailsClip::onTimelineKey(int keycode)
{
    CatchExceptions([this, keycode]
    {
        switch (keycode)
        {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                unsigned int index = keycode - '1';
                if (index < mLengthButtons.size())
                {
                    VAR_DEBUG(index);
                    handleLengthButtonPressed(mLengthButtons[index]);
                }
                break;
            }
        }
    });
}

void DetailsClip::handleLengthButtonPressed(wxToggleButton* button)
{
    ASSERT_NONZERO(button);
    ASSERT(wxThread::IsMain());
    if (!button->IsEnabled()) { return; }
    // When pressing keys in the timeline no checking is done whether a clip is already selected.
    // Furthermore, the - no clip selected - has already more than once caused crashes here.
    // So, if no clip selected, just ignore the request.
    if (mClip == nullptr) { return; }
    pts length = getLength(button);
    VAR_INFO(length);
    ASSERT(mTrimAtEnd.find(length) != mTrimAtEnd.end())(mTrimAtEnd)(length);
    ASSERT(mTrimAtBegin.find(length) != mTrimAtBegin.end())(mTrimAtBegin)(length);

    ::cmd::Combiner* command = new ::cmd::Combiner();

    getTimeline().beginTransaction();

    model::IClipPtr clip = mClip;
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
    pts endtrim = mTrimAtEnd[length];
    pts begintrim = mTrimAtBegin[length];
    bool shift = !transition;
    bool error = false;

    if (endtrim != 0)
    {
        ::gui::timeline::cmd::TrimClip* trimCommand = new cmd::TrimClip(getSequence(), clip, model::TransitionPtr(), transition ? TransitionEnd : ClipEnd);
        trimCommand->update(endtrim, shift, true);
        clip = trimCommand->getNewClip();
        transition = boost::dynamic_pointer_cast<model::Transition>(clip);
        command->add(trimCommand);
    }

    if (begintrim != 0)
    {
        if (endtrim != 0)
        {
            // When determining the trim boundaries, end trim and begin trim boundaries are determined without taking into account that both sides may be trimmed.
            // That may cause certain trim operations to cause unwanted results. Instead of applying one trim and then determining the final trim result, or trying
            // another 'trick', before doing the second trim here, an additional boundaries check is done here.
            //
            // The differences between the original computation and the result here are particularly related to shifting clips in other tracks and
            // related to having multiple transitions besides the clip.
            cmd::TrimClip::TrimLimit limitsBeginTrim;
            if (transition)
            {
                limitsBeginTrim = cmd::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionBegin, false);
            }
            else
            {
                limitsBeginTrim = cmd::TrimClip::determineBoundaries(getSequence(), clip, clip->getLink(), ClipBegin, true);
            }
            error = (begintrim < limitsBeginTrim.Min || begintrim > limitsBeginTrim.Max);
        }
        if (!error)
        {
            ::gui::timeline::cmd::TrimClip* trimCommand = new cmd::TrimClip(getSequence(), clip, model::TransitionPtr(), transition ? TransitionBegin : ClipBegin);
            trimCommand->update(begintrim, shift, true);
            clip = trimCommand->getNewClip();
            command->add(trimCommand);
        }
    }

    if (error)
    {
        gui::StatusBar::get().timedInfoText(_("Could not execute the length change without messing up clips in other tracks."));
        delete command; // Do not execute. Undo any changes.
        clip = mClip;
    }
    else
    {
        command->setName(_("Set length of clip"));
        command->submit();
    }

    // It might be possible that a new length selection button has already been pressed
    // and it's button event is already queued. When that event is handled this new clip
    // must be used.
    setClip(clip);
    //NOT: updateLengthButtons(); -- this is automatically done after selecting a new clip
    getTimeline().endTransaction();
    getTimeline().Refresh();
    getTimeline().SetFocus();
}

}} // namespace
