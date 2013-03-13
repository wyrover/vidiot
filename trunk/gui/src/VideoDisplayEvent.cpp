#include "VideoDisplayEvent.h"

namespace gui {

DEFINE_EVENT(EVENT_PLAYBACK_POSITION, PlaybackPositionEvent, long);
DEFINE_EVENT(EVENT_PLAYBACK_ACTIVE, PlaybackActiveEvent, bool);

} // namespace