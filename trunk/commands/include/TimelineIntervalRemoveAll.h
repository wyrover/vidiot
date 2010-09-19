#ifndef TIMELINE_INTERVAL_REMOVE_ALL_H
#define TIMELINE_INTERVAL_REMOVE_ALL_H

#include <wx/region.h>
#include "TimelineCommand.h"
#include "GuiPtr.h"

namespace command {

class TimelineIntervalRemoveAll 
    :   public TimelineCommand
{
public:

    /** This command removes all intervals. */
    TimelineIntervalRemoveAll(gui::timeline::IntervalsPtr intervals);

    ~TimelineIntervalRemoveAll();
    bool Do();
    bool Undo();
private:
    gui::timeline::IntervalsPtr mIntervals;
    wxRegion mOldRegion;
};

} // namespace

#endif // TIMELINE_INTERVAL_REMOVE_ALL_H
