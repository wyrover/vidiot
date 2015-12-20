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
    , mKeyFrameIndex{ boost::none }
    , mOpacity{ boost::none }
    , mScaling{ boost::none }
    , mScalingFactor{ boost::none }
    , mAlignment{ boost::none }
    , mPosition{ boost::none }
    , mRotation{ boost::none }
{
}

KeyFrame::operator bool() const
{
    model::VideoClipKeyFramePtr keyFrame{ mKeyFrameIndex ? VideoKeyFrame(mClip, *mKeyFrameIndex) : DefaultVideoKeyFrame(mClip) };
    if (mOpacity)
    {
        ASSERT_EQUALS(*mOpacity, keyFrame->getOpacity());
    }
    if (mScaling)
    {
        ASSERT_EQUALS(*mScaling, keyFrame->getScaling());
    }
    if (mScalingFactor)
    {
        ASSERT_EQUALS(*mScalingFactor, keyFrame->getScalingFactor());
    }
    if (mAlignment)
    {
        ASSERT_EQUALS(*mAlignment, keyFrame->getAlignment());
    }
    if (mPosition)
    {
        ASSERT_EQUALS(*mPosition, keyFrame->getPosition());
    }
    if (mRotation)
    {
        ASSERT_EQUALS(*mRotation, keyFrame->getRotation());
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
    boost::optional<size_t> widget_activekeyframebutton{ boost::none };
    util::thread::RunInMainAndWait([&]
    {
        wxWindow* current{ getTimeline().getDetails().getCurrent() };
        gui::timeline::DetailsClip* detailsclip{ dynamic_cast<gui::timeline::DetailsClip*>(current) };
        if (detailsclip != nullptr)
        {
            selectedclip = detailsclip->getClip();
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

            for (size_t i = 0; i < detailsclip->getVideoKeyFrameButtonCount(); ++i)
            {
                if (detailsclip->getVideoKeyFrameButton(i)->GetValue())
                {
                    ASSERT(!widget_activekeyframebutton);
                    widget_activekeyframebutton.reset(i);
                    // NOT: break; -- check that all other buttons have !GetValue()
                }
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
    if (mKeyFrameIndex)
    {
        ASSERT(widget_activekeyframebutton);
        ASSERT_EQUALS(*widget_activekeyframebutton, *mKeyFrameIndex);
    }

    return true;
}

} // namespace
