#include "TimelineCommand.h"

namespace command {

TimelineCommand::TimelineCommand(gui::timeline::Timeline& timeline)
:   RootCommand()
,   mTimeline(timeline)
{
}

TimelineCommand::~TimelineCommand()
{
}

gui::timeline::Timeline& TimelineCommand::getTimeline()
{
    return mTimeline;
}

} // namespace