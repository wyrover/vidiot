#ifndef VIDEO_DISPLAY_EVENT_H
#define VIDEO_DISPLAY_EVENT_H

#include "UtilEvent.h"

namespace gui {

DECLARE_EVENT(EVENT_PLAYBACK_POSITION, PlaybackPositionEvent, long);
DECLARE_EVENT(EVENT_PLAYBACK_ACTIVE, PlaybackActiveEvent, bool);

} // namespace

#endif // VIDEO_DISPLAY_EVENT_H