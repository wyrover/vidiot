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

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsClip::onOpacityChanged(model::EventChangeVideoClipOpacity& event)
{
    mOpacitySlider->SetValue(event.getValue());
    mOpacitySpin->SetValue(event.getValue());
    preview();
    event.Skip();
}

void DetailsClip::onScalingChanged(model::EventChangeVideoClipScaling& event)
{
    mSelectScaling->select(event.getValue());
    event.Skip();
}

void DetailsClip::onScalingFactorChanged(model::EventChangeVideoClipScalingFactor& event)
{
    mScalingSpin->SetValue(boost::rational_cast<double>(event.getValue()));
    mScalingSlider->SetValue(factorToSliderValue(event.getValue()));
    preview();
    event.Skip();
}

void DetailsClip::onRotationChanged(model::EventChangeVideoClipRotation& event)
{
    mRotationSpin->SetValue(boost::rational_cast<double>(event.getValue()));
    mRotationSlider->SetValue(floor(event.getValue() * sRotationPrecisionFactor));
    preview();
    event.Skip();
}

void DetailsClip::onAlignmentChanged(model::EventChangeVideoClipAlignment& event)
{
    mSelectAlignment->select(event.getValue());
    preview();
    event.Skip();
}

void DetailsClip::onPositionChanged(model::EventChangeVideoClipPosition& event)
{
    mPositionXSpin->SetValue(event.getValue().x);
    mPositionXSlider->SetValue(event.getValue().x);
    mPositionYSpin->SetValue(event.getValue().y);
    mPositionYSlider->SetValue(event.getValue().y);
    preview();
    event.Skip();
}

void DetailsClip::onMinPositionChanged(model::EventChangeVideoClipMinPosition& event)
{
    mPositionXSpin->SetRange(event.getValue().x,mPositionXSpin->GetMax());
    mPositionYSpin->SetRange(event.getValue().y,mPositionYSpin->GetMax());
    mPositionXSlider->SetRange(event.getValue().x,mPositionXSlider->GetMax());
    mPositionYSlider->SetRange(event.getValue().y,mPositionYSlider->GetMax());
    event.Skip();
}

void DetailsClip::onMaxPositionChanged(model::EventChangeVideoClipMaxPosition& event)
{
    mPositionXSpin->SetRange(mPositionXSpin->GetMin(),event.getValue().x);
    mPositionYSpin->SetRange(mPositionYSpin->GetMin(), event.getValue().y);
    mPositionXSlider->SetRange(mPositionXSlider->GetMin(),event.getValue().x);
    mPositionYSlider->SetRange(mPositionYSlider->GetMin(), event.getValue().y);
    event.Skip();
}

void DetailsClip::onVolumeChanged(model::EventChangeAudioClipVolume& event)
{
    mVolumeSlider->SetValue(event.getValue());
    mVolumeSpin->SetValue(event.getValue());
    event.Skip();
}

void DetailsClip::onTransitionParameterChanged(model::EventTransitionParameterChanged& event)
{
    submitEditCommandUponTransitionEdit(event.getValue());
    event.Skip();
}

void DetailsClip::onSelectionChanged(timeline::EventSelectionUpdate& event)
{
    VAR_DEBUG(this);
    CatchExceptions([this]
    {
        if (getTrim().isActive())
        {
            // At the end of the trim operation, the trim command is initialized.
            // In AClipEdit::initialize another EventSelectionUpdate is triggered.
            // Updating the current clip during trimming causes unnecessary
            // toggling (flickering between player - via DetailsClip::edit -
            // and the preview - via the trim preview).
            return;
        }
        std::set<model::IClipPtr> selection = getSequence()->getSelectedClips();
        model::IClipPtr selectedclip;
        VAR_DEBUG(selection.size());
        if (selection.size() == 1)
        {
            selectedclip = *selection.begin();
        }
        else if (selection.size() == 2)
        {
            model::IClipPtr a = *selection.begin();
            model::IClipPtr b = *(++(selection.begin()));
            if (a->getLink() == b)
            {
                ASSERT_EQUALS(b->getLink(), a);
                selectedclip = (a->isA<model::VideoClip>()) ? a : b; // Always use the videoclip (avoid problems in automated test as a result of timing differences - sometimes the videoclip is focused first, sometimes the audio clip)
            }
        }
        setClip(selectedclip);
    });
}

}} // namespace
