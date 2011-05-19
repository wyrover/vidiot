#ifndef MODEL_CLIP_EVENT_H
#define MODEL_CLIP_EVENT_H

#include "UtilEvent.h"
#include "UtilInt.h"

namespace model {

DECLARE_EVENT(EVENT_DRAG_CLIP,              EventDragClip,              bool);
DECLARE_EVENT(EVENT_SELECT_CLIP,            EventSelectClip,            bool);
DECLARE_EVENT(DEBUG_EVENT_RENDER_PROGRESS,  DebugEventRenderProgress,   pts);

} // namespace

#endif // MODEL_CLIP_EVENT_H
