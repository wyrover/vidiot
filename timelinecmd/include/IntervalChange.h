#ifndef INTERVAL_CHANGE_H
#define INTERVAL_CHANGE_H

#include "ATimelineCommand.h"

namespace gui { namespace timeline { namespace command {

class IntervalChange 
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \param Intervals the object to which the change must be made. Is a shared ptr to avoid exceptions in undo handling when closing the timeline.
    /// \param begin begin pts of interval.
    /// \param end end pts of interval
    /// \param add true if interval must be added, false if interval must be removed
    IntervalChange(gui::timeline::Timeline& timeline, long begin, long end, bool add);

    ~IntervalChange();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do();
    bool Undo();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    long mBegin;
    long mEnd;
    bool mAdd;
};

}}} // namespace

#endif // INTERVAL_CHANGE_H
