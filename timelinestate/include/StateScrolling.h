#ifndef STATE_SCROLLING_H
#define STATE_SCROLLING_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvRightUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct StateScrolling
    :   public TimeLineState< StateScrolling, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StateScrolling( my_context ctx );

    ~StateScrolling();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvRightUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvRightUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvKeyDown& evt );
};

}}} // namespace

#endif // STATE_SCROLLING_H