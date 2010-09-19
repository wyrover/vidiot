#ifndef TIMELINE_INTERVAL_CHANGE_H
#define TIMELINE_INTERVAL_CHANGE_H

#include "TimelineCommand.h"
#include "GuiPtr.h"

namespace command {

class TimelineIntervalChange : public TimelineCommand
{
public:

    /**
     * @param Intervals the object to which the change must be made. Is a shared ptr to avoid exceptions in undo handling when closing the timeline.
     * @param begin begin pts of interval.
     * @param end end pts of interval
     * @param add true if interval must be added, false if interval must be removed
     * @todo Clean up these commands when the associated timeline is destroyed. Hmmm, or make closing the timeline undo-able.
     */
    TimelineIntervalChange(gui::timeline::IntervalsPtr intervals, long begin, long end, bool add);

    ~TimelineIntervalChange();
    bool Do();
    bool Undo();
private:
    gui::timeline::IntervalsPtr mIntervals;
    long mBegin;
    long mEnd;
    bool mAdd;
};

} // namespace

#endif // TIMELINE_INTERVAL_CHANGE_H
