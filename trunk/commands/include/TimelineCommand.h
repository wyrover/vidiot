#ifndef TIMELINE_COMMAND_H
#define TIMELINE_COMMAND_H

#include "RootCommand.h"

namespace gui { namespace timeline { 
    class GuiTimeLine; 
} }

namespace command {

class TimelineCommand : public RootCommand
{
public:
    TimelineCommand(gui::timeline::GuiTimeLine& timeline);
    ~TimelineCommand();
    gui::timeline::GuiTimeLine& getTimeline();
private:
    gui::timeline::GuiTimeLine& mTimeline;
};

/** @todo upon closing the timeline, remove all related commands from the history. */

} // namespace

#endif // TIMELINE_COMMAND_H
