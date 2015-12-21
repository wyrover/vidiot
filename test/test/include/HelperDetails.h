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
    inline KeyFrameValues& Scaling(model::VideoScaling scaling) { mScaling.reset(scaling); return *this; }
    inline KeyFrameValues& ScalingFactor(rational64 scalingfactor) { mScalingFactor.reset(scalingfactor); return *this; }
    inline KeyFrameValues& Alignment(model::VideoAlignment alignment) { mAlignment.reset(alignment); return *this; }
    inline KeyFrameValues& Position(wxPoint position) { mPosition.reset(position); return *this; }
    inline KeyFrameValues& Rotation(rational64 rotation) { mRotation.reset(rotation); return *this; }

    virtual operator bool() const = 0;

protected:

    model::IClipPtr mClip = nullptr;
    boost::optional<size_t> mKeyFrameIndex;
    boost::optional<pts> mKeyFrameOffset;
    boost::optional<int> mOpacity;
    boost::optional<model::VideoScaling> mScaling;
    boost::optional<rational64> mScalingFactor;
    boost::optional<model::VideoAlignment> mAlignment;
    boost::optional<wxPoint> mPosition;
    boost::optional<rational64> mRotation;
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
    using KeyFrameValues::KeyFrameValues; // todo use this inheriting constructors pg.596 trick more often (for example for the state events)
    operator bool() const override;
};

} // namespace
