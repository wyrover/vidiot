#ifndef MODEL_VIDEOCLIP_EVENT_H
#define MODEL_VIDEOCLIP_EVENT_H

#include "UtilEvent.h"
#include "Enums.h"

namespace model {

DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALING, EventChangeVideoClipScaling, VideoScaling);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, EventChangeVideoClipScalingFactor, double);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_OFFSETX, EventChangeVideoClipOffsetX, int);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_OFFSETY, EventChangeVideoClipOffsetY, int);

} // namespace

#endif // MODEL_VIDEOCLIP_EVENT_H