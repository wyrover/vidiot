#ifndef STATE_ALWAYS_H
#define STATE_ALWAYS_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateTop.h"

namespace gui { namespace timeline { namespace state {

class Always
    :   public TimeLineState< Always, StateTop::orthogonal<1> >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Always( my_context ctx );

    ~Always();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvWheel >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvRightDown& evt );
    boost::statechart::result react( const EvWheel& evt );
};

}}} // namespace

#endif // STATE_ALWAYS_H
