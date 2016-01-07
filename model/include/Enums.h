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

#include "UtilEnum.h"

namespace model {

/// VideoScalingFitAll      : Scale such that all of the video fits in the target area.
/// VideoScalingFitToFill   : Scale such that one one fo the sizes fits exactly in the target area. In the other direction, the video either fits exactly or is larger.
/// VideoScalingNone        : Keep original size
/// VideoScalingCustom      : User specified scaling factor
DECLAREENUM(VideoScaling, \
            VideoScalingFitAll, \
            VideoScalingFitToFill, \
            VideoScalingNone, \
            VideoScalingCustom);

DECLAREENUM(VideoAlignment, \
            VideoAlignmentCenter, \
            VideoAlignmentCenterHorizontal, \
            VideoAlignmentCenterVertical, \
            VideoAlignmentCustom);

DECLAREENUM(TransitionType, \
            TransitionTypeFadeIn, \
            TransitionTypeFadeOut, \
            TransitionTypeFadeInFromPrevious, \
            TransitionTypeFadeOutToNext);

DECLAREENUM(DefaultNewProjectWizardStart, \
            DefaultNewProjectWizardStartNone, \
            DefaultNewProjectWizardStartFolder, \
            DefaultNewProjectWizardStartFiles, \
            DefaultNewProjectWizardStartBlank);

}
