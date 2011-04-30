#include "IClip.h"

namespace model {
DEFINE_EVENT(EVENT_DRAG_CLIP,               EventDragClip,              bool);
DEFINE_EVENT(EVENT_SELECT_CLIP,             EventSelectClip,            bool);
DEFINE_EVENT(DEBUG_EVENT_RENDER_PROGRESS,   DebugEventRenderProgress,   pts);
} // namespace