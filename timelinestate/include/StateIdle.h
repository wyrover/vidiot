#ifndef STATE_IDLE_H
#define STATE_IDLE_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateTop.h"

namespace gui { namespace timeline { namespace state {

class Idle
    :   public TimeLineState< Idle, StateTop::orthogonal<0> >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Idle( my_context ctx );

    ~Idle();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvDragEnter >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvRightDown& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvDragEnter& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result start();

};

}}} // namespace

#endif // STATE_IDLE_H
