#ifndef INTERVAL_REMOVE_ALL_H
#define INTERVAL_REMOVE_ALL_H

#include <wx/region.h>
#include "ATimelineCommand.h"

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

    IntervalRemoveAll(gui::timeline::Timeline& timeline);

    ~IntervalRemoveAll();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do();
    bool Undo();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxRegion mOldRegion;
};

}}} // namespace

#endif // INTERVAL_REMOVE_ALL_H
