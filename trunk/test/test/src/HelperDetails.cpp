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

#include "Test.h"

namespace test {

gui::timeline::DetailsClip* DetailsClipView()
{
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);
    return detailsclip;
}

KeyFrameValues::KeyFrameValues(model::IClipPtr clip)
    : mClip(clip)
{
}

KeyFrame::operator bool() const
{
    std::pair<pts, model::VideoKeyFramePtr> pos_frame_video{ std::make_pair(0, nullptr) };
    std::pair<pts, model::AudioKeyFramePtr> pos_frame_audio{ std::make_pair(0, nullptr) };

    pts pos{ -1 };
    if (mClip->isA<model::VideoClip>())
    {
        pos_frame_video = mKeyFrameIndex ? VideoKeyFrame(mClip, *mKeyFrameIndex) : std::make_pair(-1l, DefaultVideoKeyFrame(mClip));
        pos = pos_frame_video.first;
    }
    if (mClip->isA<model::AudioClip>())
    {
        pos_frame_audio = mKeyFrameIndex ? AudioKeyFrame(mClip, *mKeyFrameIndex) : std::make_pair(-1l, DefaultAudioKeyFrame(mClip));
        pos = pos_frame_audio.first;
    }

    model::VideoKeyFramePtr keyFrameVideo{ pos_frame_video.second };
    model::AudioKeyFramePtr keyFrameAudio{ pos_frame_audio.second };

    if (mKeyFrameOffset)
    {
        ASSERT(keyFrameVideo || keyFrameAudio);
        ASSERT_EQUALS(*mKeyFrameOffset, pos);
    }
    if (mOpacity)
    {
        ASSERT(keyFrameVideo);
        ASSERT_EQUALS(*mOpacity, keyFrameVideo->getOpacity());
    }
    if (mScaling)
    {
        ASSERT(keyFrameVideo);
        ASSERT_EQUALS(*mScaling, keyFrameVideo->getScaling());
    }
    if (mScalingFactor)
    {
        ASSERT(keyFrameVideo);
        ASSERT_EQUALS(*mScalingFactor, keyFrameVideo->getScalingFactor());
    }
    if (mAlignment)
    {
        ASSERT(keyFrameVideo);
        ASSERT_EQUALS(*mAlignment, keyFrameVideo->getAlignment());
    }
    if (mPosition)
    {
        ASSERT(keyFrameVideo);
        ASSERT_EQUALS(*mPosition, keyFrameVideo->getPosition());
    }
    if (mRotation)
    {
        ASSERT(keyFrameVideo);
        ASSERT_EQUALS(*mRotation, keyFrameVideo->getRotation());
    }
    if (mVolume)
    {
        ASSERT(keyFrameAudio);
        ASSERT_EQUALS(*mVolume, keyFrameAudio->getVolume());
    }
    return true;
}

DetailsView::operator bool() const
{
    WaitForIdle;
    model::IClipPtr selectedclip{ nullptr };
    model::VideoScaling widget_scaling{ model::VideoScalingCustom };
    int widget_opacityslider{ 0 };
    int widget_opacityspin{ 0 };
    int widget_scalingdigits{ 0 };
    double widget_scalingspin{ 0.0 };
    model::VideoAlignment widget_alignment{ model::VideoAlignmentCustom };
    int widget_xslider{ 0 };
    int widget_xspin{ 0 };
    int widget_yslider{ 0 };
    int widget_yspin{ 0 };
    int widget_rotationdigits{ 0 };
    double widget_rotationspin{ 0.0 };
    int widget_volumeslider{ 0 };
    int widget_volumespin{ 0 };
    boost::optional<size_t> widget_activekeyframebutton{ boost::none };
    bool widget_home{ false };
    bool widget_prev{ false };
    bool widget_next{ false };
    bool widget_end{ false };
    bool widget_add{ false };
    bool widget_remove{ false };
    size_t widget_buttoncount{ 0 };

    util::thread::RunInMainAndWait([&]
    {
        wxWindow* current{ getTimeline().getDetails().getCurrent() };
        gui::timeline::DetailsClip* detailsclip{ dynamic_cast<gui::timeline::DetailsClip*>(current) };
        if (detailsclip != nullptr)
        {
            selectedclip = detailsclip->getClip();
            if (mClip->isA<model::VideoClip>() != selectedclip->isA<model::VideoClip>())
            {
                selectedclip = selectedclip->getLink();
            }
            if (mClip->isA<model::VideoClip>())
            {
                widget_opacityslider = detailsclip->getOpacitySlider()->GetValue();
                widget_opacityspin = detailsclip->getOpacitySpin()->GetValue();
                widget_scaling = detailsclip->getScalingSelector()->getValue();
                widget_scalingdigits = detailsclip->getScalingSlider()->GetValue();
                widget_scalingspin = detailsclip->getScalingSpin()->GetValue();
                widget_alignment = detailsclip->getAlignmentSelector()->getValue();
                widget_xslider = detailsclip->getPositionXSlider()->GetValue();
                widget_xspin = detailsclip->getPositionXSpin()->GetValue();
                widget_yslider = detailsclip->getPositionYSlider()->GetValue();
                widget_yspin = detailsclip->getPositionYSpin()->GetValue();
                widget_rotationdigits = detailsclip->getRotationSlider()->GetValue();
                widget_rotationspin = detailsclip->getRotationSpin()->GetValue();

                widget_buttoncount = detailsclip->getVideoKeyFrameButtonCount();
                for (size_t i = 0; i < widget_buttoncount; ++i)
                {
                    if (detailsclip->getVideoKeyFrameButton(i)->GetValue())
                    {
                        ASSERT(!widget_activekeyframebutton);
                        widget_activekeyframebutton.reset(i);
                        // NOT: break; -- check that all other buttons have !GetValue()
                    }
                }

                widget_home = detailsclip->getVideoKeyFramesHomeButton()->IsEnabled();
                widget_prev = detailsclip->getVideoKeyFramesPrevButton()->IsEnabled();
                widget_next = detailsclip->getVideoKeyFramesNextButton()->IsEnabled();
                widget_end = detailsclip->getVideoKeyFramesEndButton()->IsEnabled();
                widget_add = detailsclip->getVideoKeyFramesAddButton()->IsEnabled();
                widget_remove = detailsclip->getVideoKeyFramesRemoveButton()->IsEnabled();
            }
            else
            {
                widget_volumeslider = detailsclip->getVolumeSlider()->GetValue();
                widget_volumespin = detailsclip->getVolumeSpin()->GetValue();

                widget_buttoncount = detailsclip->getAudioKeyFrameButtonCount();
                for (size_t i = 0; i < widget_buttoncount; ++i)
                {
                    if (detailsclip->getAudioKeyFrameButton(i)->GetValue())
                    {
                        ASSERT(!widget_activekeyframebutton);
                        widget_activekeyframebutton.reset(i);
                        // NOT: break; -- check that all other buttons have !GetValue()
                    }
                }

                widget_home = detailsclip->getAudioKeyFramesHomeButton()->IsEnabled();
                widget_prev = detailsclip->getAudioKeyFramesPrevButton()->IsEnabled();
                widget_next = detailsclip->getAudioKeyFramesNextButton()->IsEnabled();
                widget_end = detailsclip->getAudioKeyFramesEndButton()->IsEnabled();
                widget_add = detailsclip->getAudioKeyFramesAddButton()->IsEnabled();
                widget_remove = detailsclip->getAudioKeyFramesRemoveButton()->IsEnabled();
            }
        }
    });

    ASSERT_EQUALS(selectedclip, mClip);

    if (mOpacity)
    {
        ASSERT_EQUALS(*mOpacity, widget_opacityslider);
        ASSERT_EQUALS(*mOpacity, widget_opacityspin);
    }
    if (mScaling)
    {
        ASSERT_EQUALS(*mScaling, widget_scaling);
    }
    if (mScalingFactor)
    {
        ASSERT_EQUALS(*mScalingFactor, gui::timeline::DetailsClip::sliderValueToFactor(widget_scalingdigits));
        ASSERT_EQUALS(floor(boost::rational_cast<double>(*mScalingFactor) * 100), floor(widget_scalingspin * 100)); // floor + *100 : ensure that only two digits are used
    }
    if (mAlignment)
    {
        ASSERT_EQUALS(*mAlignment, widget_alignment);
    }
    if (mPosition)
    {
        ASSERT_EQUALS(mPosition->x, widget_xslider)(widget_yslider);
        ASSERT_EQUALS(mPosition->x, widget_xspin);
        ASSERT_EQUALS(mPosition->y, widget_yslider)(widget_xslider);
        ASSERT_EQUALS(mPosition->y, widget_yspin);
    }
    if (mRotation)
    {
        int rotationdigits{ boost::rational_cast<int>(*mRotation * gui::timeline::DetailsClip::sRotationPrecisionFactor) };
        ASSERT_EQUALS(rotationdigits, widget_rotationdigits);
        ASSERT_EQUALS(floor(boost::rational_cast<double>(*mRotation) * 100), floor(widget_rotationspin * 100)); // floor + *100 : ensure that only two digits are used
    }
    if (mVolume)
    {
        ASSERT_EQUALS(*mVolume, widget_volumeslider);
        ASSERT_EQUALS(*mVolume, widget_volumespin);
    }
    if (mKeyFrameIndex)
    {
        ASSERT(widget_activekeyframebutton);
        ASSERT_EQUALS(*widget_activekeyframebutton, *mKeyFrameIndex);
    }
    if (mNoKeyFrame)
    {
        ASSERT(!widget_activekeyframebutton);
    }
    if (mKeyframeHomeButton)
    {
        ASSERT_EQUALS(*mKeyframeHomeButton, widget_home);
    }
    if (mKeyframePrevButton)
    {
        ASSERT_EQUALS(*mKeyframePrevButton, widget_prev);
    }
    if (mKeyframeNextButton)
    {
        ASSERT_EQUALS(*mKeyframeNextButton, widget_next);
    }
    if (mKeyframeEndButton)
    {
        ASSERT_EQUALS(*mKeyframeEndButton, widget_end);
    }
    if (mKeyframeAddButton)
    {
        ASSERT_EQUALS(*mKeyframeAddButton, widget_add);
    }
    if (mKeyframeRemoveButton)
    {
        ASSERT_EQUALS(*mKeyframeRemoveButton, widget_remove);
    }
    if (mKeyframeCount)
    {
        ASSERT_EQUALS(*mKeyframeCount, widget_buttoncount);
    }
    return true;
}

} // namespace
