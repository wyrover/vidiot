#include "VideoClipEvent.h"

namespace model {

DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALING, EventChangeVideoClipScaling, VideoScaling);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, EventChangeVideoClipScalingFactor, double);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_OFFSETX, EventChangeVideoClipOffsetX, int);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_OFFSETY, EventChangeVideoClipOffsetY, int);

} // namespace