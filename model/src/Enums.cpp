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

#include "Enums.h"

#include "Config.h"

namespace model {

IMPLEMENTENUM(VideoScaling);

IMPLEMENTENUM(VideoAlignment);

IMPLEMENTENUM(TransitionType);

IMPLEMENTENUM(DefaultNewProjectWizardStart);

std::map<VideoScaling, wxString> VideoScalingConverter::getMapToHumanReadibleString()
{
    return
    {
        { VideoScalingFitAll, _("Fit all") },
        { VideoScalingFitToFill, _("Fit to fill") },
        { VideoScalingNone, _("Original size") },
        { VideoScalingCustom, _("Custom") },
        { VideoScalingHalf, _("1/2") },
        { VideoScalingThird, _("1/3") },
        { VideoScalingFourth, _("1/4") },
    };
};

std::map<VideoAlignment, wxString> VideoAlignmentConverter::getMapToHumanReadibleString()
{
    return
    {
        { VideoAlignmentCenter, _("Centered") },
        { VideoAlignmentCenterHorizontal, _("Centered horizontally") },
        { VideoAlignmentCenterVertical, _("Centered vertically") },
        { VideoAlignmentCustom, _("Custom") },                 
        { VideoAlignmentTopLeft, _("Top left") },
        { VideoAlignmentTop, _("Top") },         
        { VideoAlignmentTopCenter, _("Top center") },
        { VideoAlignmentTopRight, _("Top right") },
        { VideoAlignmentRight, _("Right") },
        { VideoAlignmentRightCenter, _("Right center") },
        { VideoAlignmentBottomRight, _("Bottom right") },
        { VideoAlignmentBottom, _("Bottom") },
        { VideoAlignmentBottomCenter, _("Bottom centered") },
        { VideoAlignmentBottomLeft, _("Bottom left") },
        { VideoAlignmentLeft, _("Left") },
        { VideoAlignmentLeftCenter, _("Left centered") },
    };
}

}