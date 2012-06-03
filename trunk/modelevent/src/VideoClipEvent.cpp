#include "VideoClipEvent.h"

#include "UtilLogWxwidgets.h"

namespace model {

DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_OPACITY,       EventChangeVideoClipOpacity,       int);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALING,       EventChangeVideoClipScaling,       VideoScaling);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_SCALINGDIGITS, EventChangeVideoClipScalingDigits, int);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_ALIGNMENT,     EventChangeVideoClipAlignment,     VideoAlignment);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_POSITION,      EventChangeVideoClipPosition,      wxPoint);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_MINPOSITION,   EventChangeVideoClipMinPosition,   wxPoint);
DEFINE_EVENT(EVENT_CHANGE_VIDEOCLIP_MAXPOSITION,   EventChangeVideoClipMaxPosition,   wxPoint);

} // namespace