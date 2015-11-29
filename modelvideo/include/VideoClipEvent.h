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

#include "UtilEvent.h"
#include "Enums.h"

namespace model {

DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_OPACITY,       EventChangeVideoClipOpacity,       int);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALING,       EventChangeVideoClipScaling,       VideoScaling);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, EventChangeVideoClipScalingFactor, rational64);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_ROTATION,      EventChangeVideoClipRotation,      rational64);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_ALIGNMENT,     EventChangeVideoClipAlignment,     VideoAlignment);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_POSITION,      EventChangeVideoClipPosition,      wxPoint);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_MINPOSITION,   EventChangeVideoClipMinPosition,   wxPoint);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_MAXPOSITION,   EventChangeVideoClipMaxPosition,   wxPoint);

} // namespace
