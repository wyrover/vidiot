#ifndef STATE_ALWAYS_H
#define STATE_ALWAYS_H

#include <boost/statechart/custom_reaction.hpp>
#include "State.h"

namespace gui { namespace timeline { namespace state {

class Always
    :   public TimeLineStateInner< Always, Machine, Idle >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Always( my_context ctx );

    ~Always();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvWheel >,
        boost::statechart::custom_reaction< EvLeave >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvWheel& evt );
    boost::statechart::result react( const EvLeave& evt);
};

}}} // namespace

#endif // STATE_ALWAYS_H
