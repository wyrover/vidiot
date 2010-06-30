#ifndef TIMELINE_CREATE_VIDEO_TRACK_H
#define TIMELINE_CREATE_VIDEO_TRACK_H

#include "TimelineCommand.h"
#include "Sequence.h"
#include "VideoTrack.h"

namespace command {

class TimelineCreateVideoTrack : public TimelineCommand
{
public:
    TimelineCreateVideoTrack(model::SequencePtr sequence);
    ~TimelineCreateVideoTrack();
    bool Do();
    bool Undo();
private:
    model::SequencePtr mSequence;
    model::VideoTrackPtr mNewTrack;
};

} // namespace

#endif // TIMELINE_CREATE_VIDEO_TRACK_H
