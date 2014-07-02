// Copyright 2013,2014 Eric Raijmakers.
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

#include "Details.h"
#include "DetailsClip.h"

namespace test {

gui::timeline::DetailsClip* DetailsClipView()
{
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);
    return detailsclip;
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
    RunInMainAndWait([clip,scaling,scalingfactor,alignment,position,rotation]
    {
        model::VideoClipPtr videoclip = getVideoClip(clip);
        ASSERT_DETAILSCLIP(clip);
        int scalingdigits = boost::rational_cast<int>(scalingfactor * model::Constants::sScalingPrecisionFactor);
        int rotationdigits = boost::rational_cast<int>(rotation * model::Constants::sRotationPrecisionFactor);
        ASSERT_EQUALS(DetailsClipView()->getScalingSlider()->GetValue(), scalingdigits );
        ASSERT_EQUALS(floor(DetailsClipView()->getScalingSpin()->GetValue() * model::Constants::sScalingPrecisionFactor), scalingdigits);
        ASSERT_EQUALS(DetailsClipView()->getPositionXSlider()->GetValue(),position.x)(DetailsClipView()->getPositionYSlider()->GetValue());
        ASSERT_EQUALS(DetailsClipView()->getPositionXSpin()->GetValue(),position.x);
        ASSERT_EQUALS(DetailsClipView()->getPositionYSlider()->GetValue(),position.y);
        ASSERT_EQUALS(DetailsClipView()->getPositionYSpin()->GetValue(),position.y);
        ASSERT_EQUALS(DetailsClipView()->getRotationSlider()->GetValue(), rotationdigits);
        ASSERT_EQUALS(videoclip->getScaling(),scaling);
        ASSERT_EQUALS(videoclip->getScalingFactor(),scalingfactor);
        ASSERT_EQUALS(videoclip->getAlignment(),alignment);
        ASSERT_EQUALS(videoclip->getPosition(),position);
        ASSERT_EQUALS(videoclip->getRotation(),rotation);
    });
};

} // namespace
