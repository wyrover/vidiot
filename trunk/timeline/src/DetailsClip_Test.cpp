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

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

std::vector<wxToggleButton*> DetailsClip::getLengthButtons() const
{
    return mLengthButtons;
}

wxSlider* DetailsClip::getSpeedSlider() const
{
    return mSpeedSlider;
}

wxSpinCtrlDouble* DetailsClip::getSpeedSpin() const
{
    return mSpeedSpin;
}

wxToggleButton* DetailsClip::getPlayButton() const
{
    return mPlayButton;
}

model::TransitionPtr DetailsClip::getTransitionClone() const
{
    return mTransitionClone;
}

wxChoice* DetailsClip::getTransitionTypeSelector() const
{
    return mTransitionType;
}

wxSlider* DetailsClip::getOpacitySlider() const
{
    return mOpacitySlider;
}

wxSpinCtrl* DetailsClip::getOpacitySpin() const
{
    return mOpacitySpin;
}

EnumSelector<model::VideoScaling>* DetailsClip::getScalingSelector() const
{
    return mSelectScaling;
}

wxSlider* DetailsClip::getScalingSlider() const
{
    return mScalingSlider;
}

wxSpinCtrlDouble* DetailsClip::getScalingSpin() const
{
    return mScalingSpin;
}

wxSlider* DetailsClip::getRotationSlider() const
{
    return mRotationSlider;
}

wxSpinCtrlDouble* DetailsClip::getRotationSpin() const
{
    return mRotationSpin;
}

EnumSelector<model::VideoAlignment>* DetailsClip::getAlignmentSelector() const
{
    return mSelectAlignment;
}

wxSlider* DetailsClip::getPositionXSlider() const
{
    return mPositionXSlider;
}

wxSpinCtrl* DetailsClip::getPositionXSpin() const
{
    return mPositionXSpin;
}

wxSlider* DetailsClip::getPositionYSlider() const
{
    return mPositionYSlider;
}

wxSpinCtrl* DetailsClip::getPositionYSpin() const
{
    return mPositionYSpin;
}

wxButton* DetailsClip::getVideoKeyFramesHomeButton() const 
{ 
    return mVideoKeyFrameControls->mHomeButton; 
}

wxButton* DetailsClip::getVideoKeyFramesPrevButton() const 
{ 
    return mVideoKeyFrameControls->mPrevButton;
}

wxButton* DetailsClip::getVideoKeyFramesNextButton() const 
{ 
    return mVideoKeyFrameControls->mNextButton;
}

wxButton* DetailsClip::getVideoKeyFramesEndButton() const 
{ 
    return mVideoKeyFrameControls->mEndButton;
}

wxButton* DetailsClip::getVideoKeyFramesAddButton() const 
{ 
    return mVideoKeyFrameControls->mAddButton;
}

wxButton* DetailsClip::getVideoKeyFramesRemoveButton() const 
{ 
    return mVideoKeyFrameControls->mRemoveButton;
}

size_t DetailsClip::getVideoKeyFrameButtonCount() const
{
    return mVideoKeyFrameControls->mKeyFramesButtons.size();
}

wxToggleButton* DetailsClip::getVideoKeyFrameButton(size_t index) const 
{ 
    ASSERT_MAP_CONTAINS(mVideoKeyFrameControls->mKeyFramesButtons, index);
    return mVideoKeyFrameControls->mKeyFramesButtons.find(index)->second;
}

wxSlider* DetailsClip::getVolumeSlider() const
{
    return mVolumeSlider;
}

wxSpinCtrl* DetailsClip::getVolumeSpin() const
{
    return mVolumeSpin;
}

wxButton* DetailsClip::getAudioKeyFramesHomeButton() const
{
    return mAudioKeyFrameControls->mHomeButton;
}

wxButton* DetailsClip::getAudioKeyFramesPrevButton() const
{
    return mAudioKeyFrameControls->mPrevButton;
}

wxButton* DetailsClip::getAudioKeyFramesNextButton() const
{
    return mAudioKeyFrameControls->mNextButton;
}

wxButton* DetailsClip::getAudioKeyFramesEndButton() const
{
    return mAudioKeyFrameControls->mEndButton;
}

wxButton* DetailsClip::getAudioKeyFramesAddButton() const
{
    return mAudioKeyFrameControls->mAddButton;
}

wxButton* DetailsClip::getAudioKeyFramesRemoveButton() const
{
    return mAudioKeyFrameControls->mRemoveButton;
}

size_t DetailsClip::getAudioKeyFrameButtonCount() const
{
    return mAudioKeyFrameControls->mKeyFramesButtons.size();
}

wxToggleButton* DetailsClip::getAudioKeyFrameButton(size_t index) const
{
    ASSERT_MAP_CONTAINS(mAudioKeyFrameControls->mKeyFramesButtons, index);
    return mAudioKeyFrameControls->mKeyFramesButtons.find(index)->second;
}

}} // namespace
