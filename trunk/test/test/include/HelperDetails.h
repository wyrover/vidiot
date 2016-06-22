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

#pragma once

#include "Enums.h"

namespace gui { namespace timeline {
    class DetailsClip;
}}

namespace test {

gui::timeline::DetailsClip* DetailsClipView();

struct KeyFrameValues
{
    explicit KeyFrameValues(model::IClipPtr clip);

    inline KeyFrameValues& KeyFrameIndex(size_t keyFrameIndex) { mKeyFrameIndex.reset(keyFrameIndex); return *this; };
    inline KeyFrameValues& KeyFrameOffset(pts offset) { mKeyFrameOffset.reset(offset); return *this; };
    inline KeyFrameValues& Opacity(int opacity) { mOpacity.reset(opacity); return *this; }
    inline KeyFrameValues& CropTop(int crop) { mCropTop.reset(crop); return *this; }
    inline KeyFrameValues& CropBottom(int crop) { mCropBottom.reset(crop); return *this; }
    inline KeyFrameValues& CropLeft(int crop) { mCropLeft.reset(crop); return *this; }
    inline KeyFrameValues& CropRight(int crop) { mCropRight.reset(crop); return *this; }
    inline KeyFrameValues& Scaling(model::VideoScaling scaling) { mScaling.reset(scaling); return *this; }
    inline KeyFrameValues& ScalingFactor(rational64 scalingfactor) { mScalingFactor.reset(scalingfactor); return *this; }
    inline KeyFrameValues& Alignment(model::VideoAlignment alignment) { mAlignment.reset(alignment); return *this; }
    inline KeyFrameValues& Position(wxPoint position) { mPosition.reset(position); return *this; }
    inline KeyFrameValues& Rotation(rational64 rotation) { mRotation.reset(rotation); return *this; }
    inline KeyFrameValues& Volume(int volume) { mVolume.reset(volume); return *this; };

    virtual operator bool() const = 0;

protected:

    model::IClipPtr mClip = nullptr;
    boost::optional<size_t> mKeyFrameIndex = boost::none;
    boost::optional<pts> mKeyFrameOffset = boost::none;
    boost::optional<int> mOpacity = boost::none;
    boost::optional<int> mCropTop = boost::none;
    boost::optional<int> mCropBottom = boost::none;
    boost::optional<int> mCropLeft = boost::none;
    boost::optional<int> mCropRight = boost::none;
    boost::optional<model::VideoScaling> mScaling = boost::none;
    boost::optional<rational64> mScalingFactor = boost::none;
    boost::optional<model::VideoAlignment> mAlignment = boost::none;
    boost::optional<wxPoint> mPosition = boost::none;
    boost::optional<rational64> mRotation = boost::none;
    boost::optional<int> mVolume = boost::none;
};

struct KeyFrame : public KeyFrameValues
{
    using KeyFrameValues::KeyFrameValues;
    operator bool() const override;
};

struct DefaultKeyFrame : public KeyFrame 
{ 
    using KeyFrame::KeyFrame;
};

struct DetailsView : public KeyFrameValues
{
    using KeyFrameValues::KeyFrameValues;
    operator bool() const override;

    inline DetailsView& VHome(bool enabled) { mKeyframeHomeButton.reset(enabled); return *this; };
    inline DetailsView& VPrev(bool enabled) { mKeyframePrevButton.reset(enabled); return *this; };
    inline DetailsView& VNext(bool enabled) { mKeyframeNextButton.reset(enabled); return *this; };
    inline DetailsView& VEnd(bool enabled) { mKeyframeEndButton.reset(enabled); return *this; };
    inline DetailsView& VAdd(bool enabled) { mKeyframeAddButton.reset(enabled); return *this; };
    inline DetailsView& VRemove(bool enabled) { mKeyframeRemoveButton.reset(enabled); return *this; };
    inline DetailsView& VCount(size_t count) { mKeyframeCount.reset(count); return *this; };
    inline DetailsView& NoKeyframe() { mNoKeyFrame.reset(true); return *this; };

private:

    boost::optional<bool> mKeyframeHomeButton = boost::none;
    boost::optional<bool> mKeyframePrevButton = boost::none;
    boost::optional<bool> mKeyframeNextButton = boost::none;
    boost::optional<bool> mKeyframeEndButton = boost::none;
    boost::optional<bool> mKeyframeAddButton = boost::none;
    boost::optional<bool> mKeyframeRemoveButton = boost::none;
    boost::optional<size_t> mKeyframeCount = boost::none;
    boost::optional <bool> mNoKeyFrame = boost::none;

};

} // namespace
