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

void ASSERT_CLIPPROPERTIES(
    model::IClipPtr clip,
    model::VideoScaling scaling,
    boost::rational<int> scalingfactor,
    model::VideoAlignment alignment,
    wxPoint position,
    boost::rational<int> rotation)
{
    WaitForIdle;

    model::IClipPtr selectedclip = nullptr;
    int widget_scalingdigits = 0;
    double widget_scalingspin = 0.0;
    int widget_xslider = 0;
    int widget_xspin = 0;
    int widget_yslider = 0;
    int widget_yspin = 0;
    int widget_rotationdigits = 0;

    util::thread::RunInMainAndWait([&]
    {
        selectedclip = DetailsClipView()->getClip();
        widget_scalingdigits = DetailsClipView()->getScalingSlider()->GetValue();
        widget_scalingspin = DetailsClipView()->getScalingSpin()->GetValue();
        widget_xslider = DetailsClipView()->getPositionXSlider()->GetValue();
        widget_xspin = DetailsClipView()->getPositionXSpin()->GetValue();
        widget_yslider = DetailsClipView()->getPositionYSlider()->GetValue();
        widget_yspin = DetailsClipView()->getPositionYSpin()->GetValue();
        widget_rotationdigits = DetailsClipView()->getRotationSlider()->GetValue();
    });
    ASSERT_EQUALS(selectedclip, clip);

    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(widget_scalingdigits), scalingfactor);
    ASSERT_EQUALS(floor(widget_scalingspin * 100), floor(boost::rational_cast<double>(scalingfactor) * 100)); // floor + *100 : ensure that only two digits are used
    ASSERT_EQUALS(widget_xslider,position.x)(widget_yslider);
    ASSERT_EQUALS(widget_xspin,position.x);
    ASSERT_EQUALS(widget_yslider,position.y)(widget_xslider);
    ASSERT_EQUALS(widget_yspin,position.y);
    int rotationdigits = boost::rational_cast<int>(rotation * gui::timeline::DetailsClip::sRotationPrecisionFactor);
    ASSERT_EQUALS(widget_rotationdigits, rotationdigits);

    model::VideoClipPtr videoclip = getVideoClip(clip);
    ASSERT_EQUALS(videoclip->getScaling(),scaling);
    ASSERT_EQUALS(videoclip->getScalingFactor(),scalingfactor);
    ASSERT_EQUALS(videoclip->getAlignment(),alignment);
    ASSERT_EQUALS(videoclip->getPosition(),position);
    ASSERT_EQUALS(videoclip->getRotation(),rotation);
};

} // namespace
