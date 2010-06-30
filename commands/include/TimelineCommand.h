#ifndef TIMELINE_COMMAND_H
#define TIMELINE_COMMAND_H

#include "RootCommand.h"

namespace command {

class TimelineCommand : public RootCommand
{
public:
    TimelineCommand();
    ~TimelineCommand();
};

} // namespace

#endif // TIMELINE_COMMAND_H
