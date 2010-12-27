#ifndef CREATE_VIDEO_TRACK_H
#define CREATE_VIDEO_TRACK_H

#include "ATimelineCommand.h"
#include "ModelPtr.h"

namespace gui { namespace timeline { namespace command {

class CreateVideoTrack
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CreateVideoTrack(gui::timeline::Timeline& timeline);
    ~CreateVideoTrack();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do();
    bool Undo();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::VideoTrackPtr mNewTrack;
};

}}} // namespace

#endif // TIMELINE_CREATE_VIDEO_TRACK_H
