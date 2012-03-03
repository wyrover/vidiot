#ifndef INTERVAL_CHANGE_H
#define INTERVAL_CHANGE_H

#include "ATimelineCommand.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace command {

class IntervalChange
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \param Intervals the object to which the change must be made. Is a shared ptr to avoid exceptions in undo handling when closing the timeline.
    /// \param interval interval to be added/removed
    /// \param add true if interval must be added, false if interval must be removed
    IntervalChange(model::SequencePtr sequence, PtsInterval interval, bool add);

    ~IntervalChange();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PtsInterval mInterval;
    bool mAdd;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const IntervalChange& obj );
};

}}} // namespace

#endif // INTERVAL_CHANGE_H