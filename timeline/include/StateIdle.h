#ifndef STATE_IDLE_H
#define STATE_IDLE_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"

namespace gui { namespace timeline { namespace state {

class Idle
    :   public TimeLineState< Idle, Always >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Idle( my_context ctx );

    ~Idle();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result start();

};

}}} // namespace

#endif // STATE_IDLE_H
