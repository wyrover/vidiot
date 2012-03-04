#ifndef INTERVAL_REMOVE_ALL_H
#define INTERVAL_REMOVE_ALL_H

#include "ATimelineCommand.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace command {

/// This command removes all marked intervals from the timeline.
/// This does not change the sequence, only the marked range is changed.
class IntervalRemoveAll
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    IntervalRemoveAll(model::SequencePtr sequence);

    ~IntervalRemoveAll();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PtsIntervals mIntervals;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const IntervalRemoveAll& obj );
};

}}} // namespace

#endif // INTERVAL_REMOVE_ALL_H