#ifndef MODEL_VIDEOCLIP_EVENT_H
#define MODEL_VIDEOCLIP_EVENT_H

#include "UtilEvent.h"
#include "Enums.h"

namespace model {

DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALING,       EventChangeVideoClipScaling,       VideoScaling);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALINGDIGITS, EventChangeVideoClipScalingDigits, int);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_ALIGNMENT,     EventChangeVideoClipAlignment,     VideoAlignment);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_POSITION,      EventChangeVideoClipPosition,      wxPoint);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_MINPOSITION,   EventChangeVideoClipMinPosition,   wxPoint);
DECLARE_EVENT(EVENT_CHANGE_VIDEOCLIP_MAXPOSITION,   EventChangeVideoClipMaxPosition,   wxPoint);

} // namespace

#endif // MODEL_VIDEOCLIP_EVENT_H