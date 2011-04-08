#ifndef CREATE_VIDEO_TRACK_H
#define CREATE_VIDEO_TRACK_H

#include <boost/shared_ptr.hpp>
#include "ATimelineCommand.h"

namespace model {
class VideoTrack;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
}

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
