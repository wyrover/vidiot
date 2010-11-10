#include "TimelineCommand.h"

namespace command {

TimelineCommand::TimelineCommand(gui::timeline::GuiTimeLine& timeline)
:   RootCommand()
,   mTimeline(timeline)
{
}

TimelineCommand::~TimelineCommand()
{
}

gui::timeline::GuiTimeLine& TimelineCommand::getTimeline()
{
    return mTimeline;
}

} // namespace