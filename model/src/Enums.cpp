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
    };
}

}