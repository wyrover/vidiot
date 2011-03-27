#ifndef STATE_TOP_H
#define STATE_TOP_H

#include <boost/statechart/custom_reaction.hpp>
#include "State.h"

namespace gui { namespace timeline { namespace state {

class Always;
class Idle;

/// Topmost class for containing the orthogonal submachines
class StateTop
    :   public TimeLineStateInner< StateTop, Machine, boost::mpl::list< Idle, Always > >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StateTop( my_context ctx );

    ~StateTop();

};

}}} // namespace

#endif // STATE_TOP_H
