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

void ASSERT_NO_DETAILSCLIP()
{
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_ZERO(detailsclip);
}

void ASSERT_DETAILSCLIP(model::IClipPtr clip)
{
    ASSERT_EQUALS(DetailsClipView()->getClip(),clip);
}

KeyFrameValues::KeyFrameValues(model::IClipPtr clip)
    : mClip(clip)
    , mOpacity{ boost::none }
    , mScaling{ boost::none }
    , mScalingFactor{ boost::none }
    , mAlignment{ boost::none }
    , mPosition{ boost::none }
    , mRotation{ boost::none }
{
}

KeyFrame::KeyFrame(model::IClipPtr clip, int keyFrameIndex)
    : KeyFrameValues{ clip }
    , mKeyFrame{ VideoKeyFrame(clip, narrow_cast<size_t, int>(keyFrameIndex)) }
{
}

KeyFrame::KeyFrame(model::IClipPtr clip)
    : KeyFrameValues{ clip }
    , mKeyFrame{ DefaultVideoKeyFrame(clip) }
{
}

KeyFrame::operator bool() const
{
    if (mOpacity)
    {
        ASSERT_EQUALS(*mOpacity, mKeyFrame->getOpacity());
    }
    if (mScaling)
    {
        ASSERT_EQUALS(*mScaling, mKeyFrame->getScaling());
    }
    if (mScalingFactor)
    {
        ASSERT_EQUALS(*mScalingFactor, mKeyFrame->getScalingFactor());
    }
    if (mAlignment)
    {
        ASSERT_EQUALS(*mAlignment, mKeyFrame->getAlignment());
    }
    if (mPosition)
    {
        ASSERT_EQUALS(*mPosition, mKeyFrame->getPosition());
    }
    if (mRotation)
    {
        ASSERT_EQUALS(*mRotation, mKeyFrame->getRotation());
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
    util::thread::RunInMainAndWait([&]
    {
        wxWindow* current = getTimeline().getDetails().getCurrent();
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


    return true;
}

// todo replace the assert below (at least partially) with ASSERT(KeyFrameProperties)
void ASSERT_CLIPPROPERTIES(
    model::IClipPtr clip,
    model::VideoScaling scaling,
    rational64 scalingfactor,
    model::VideoAlignment alignment,
    wxPoint position,
    rational64 rotation,
    int keyFrameIndex)
{
    WaitForIdle;

    model::IClipPtr selectedclip = nullptr;
    model::VideoScaling widget_scaling = model::VideoScalingCustom;
    int widget_scalingdigits = 0;
    double widget_scalingspin = 0.0;
    model::VideoAlignment widget_alignment = model::VideoAlignmentCustom;
    int widget_xslider = 0;
    int widget_xspin = 0;
    int widget_yslider = 0;
    int widget_yspin = 0;
    int widget_rotationdigits = 0;

    util::thread::RunInMainAndWait([&]
    {
        selectedclip = DetailsClipView()->getClip();
        widget_scaling = DetailsClipView()->getScalingSelector()->getValue();
        widget_scalingdigits = DetailsClipView()->getScalingSlider()->GetValue();
        widget_scalingspin = DetailsClipView()->getScalingSpin()->GetValue();
        widget_alignment = DetailsClipView()->getAlignmentSelector()->getValue();
        widget_xslider = DetailsClipView()->getPositionXSlider()->GetValue();
        widget_xspin = DetailsClipView()->getPositionXSpin()->GetValue();
        widget_yslider = DetailsClipView()->getPositionYSlider()->GetValue();
        widget_yspin = DetailsClipView()->getPositionYSpin()->GetValue();
        widget_rotationdigits = DetailsClipView()->getRotationSlider()->GetValue();
    });
    ASSERT_EQUALS(selectedclip, clip);

    // todo opacity
    ASSERT_EQUALS(widget_scaling, scaling);
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(widget_scalingdigits), scalingfactor);
    ASSERT_EQUALS(floor(widget_scalingspin * 100), floor(boost::rational_cast<double>(scalingfactor) * 100)); // floor + *100 : ensure that only two digits are used
    ASSERT_EQUALS(widget_alignment, alignment);
    ASSERT_EQUALS(widget_xslider, position.x)(widget_yslider);
    ASSERT_EQUALS(widget_xspin, position.x);
    ASSERT_EQUALS(widget_yslider, position.y)(widget_xslider);
    ASSERT_EQUALS(widget_yspin, position.y);
    int rotationdigits = boost::rational_cast<int>(rotation * gui::timeline::DetailsClip::sRotationPrecisionFactor);
    ASSERT_EQUALS(widget_rotationdigits, rotationdigits);

    model::VideoClipKeyFramePtr keyframe{ nullptr };
    if (keyFrameIndex == -1)
    {
        keyframe = DefaultVideoKeyFrame(clip);
    }
    else
    {
        size_t index{ narrow_cast<size_t, int>(keyFrameIndex) };
        keyframe = VideoKeyFrame(clip, index);
    }
    ASSERT_NONZERO(keyframe);
    ASSERT_EQUALS(keyframe->getScaling(), scaling);
    ASSERT_EQUALS(keyframe->getScalingFactor(), scalingfactor);
    ASSERT_EQUALS(keyframe->getAlignment(), alignment);
    ASSERT_EQUALS(keyframe->getPosition(), position);
    ASSERT_EQUALS(keyframe->getRotation(), rotation);
};

} // namespace
