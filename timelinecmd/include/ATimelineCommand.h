#ifndef TIMELINE_COMMAND_H
#define TIMELINE_COMMAND_H

#include "RootCommand.h"

namespace gui { namespace timeline { 
    class Timeline; 
}}

namespace gui { namespace timeline { namespace command {

/// Abstract base for all modifications on the timeline/sequence.
/// \todo upon closing the timeline, remove all related commands from the history.
class ATimelineCommand 
    :   public ::command::RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ATimelineCommand(gui::timeline::Timeline& timeline);
    virtual ~ATimelineCommand();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    gui::timeline::Timeline& getTimeline();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    gui::timeline::Timeline& mTimeline;
};

}}} // namespace

#endif // TIMELINE_COMMAND_H
