#ifndef TIMELINE_COMMAND_H
#define TIMELINE_COMMAND_H

#include "RootCommand.h"

namespace gui { namespace timeline { 
    class Timeline; 
} }

namespace command {

class TimelineCommand : public RootCommand
{
public:
    TimelineCommand(gui::timeline::Timeline& timeline);
    ~TimelineCommand();
    gui::timeline::Timeline& getTimeline();
private:
    gui::timeline::Timeline& mTimeline;
};

/** @todo upon closing the timeline, remove all related commands from the history. */

} // namespace

#endif // TIMELINE_COMMAND_H
