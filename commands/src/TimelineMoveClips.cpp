#include "TimelineMoveClips.h"
#include <boost/foreach.hpp>
#include "UtilLog.h"

namespace command {

TimelineMoveClips::TimelineMoveClips()
:   TimelineCommand()
{
    VAR_INFO(this);
     mCommandName = _("Move ");
}

TimelineMoveClips::~TimelineMoveClips()
{
}

bool TimelineMoveClips::Do()
{
    VAR_INFO(this);
    return true;
}

bool TimelineMoveClips::Undo()
{
    VAR_INFO(this);
    return true;
}

} // namespace