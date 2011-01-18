#ifndef STATE_MOVE_DIVIDER_H
#define STATE_MOVE_DIVIDER_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"

namespace gui { namespace timeline { namespace state {

class MoveDivider
    :   public TimeLineState< MoveDivider, Always >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    MoveDivider( my_context ctx );

    ~MoveDivider();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >,
        boost::statechart::custom_reaction< EvLeave >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvKeyUp& evt);
    boost::statechart::result react( const EvLeave& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mOriginalPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result abort();
};

}}} // namespace

#endif // STATE_MOVE_DIVIDER_H




