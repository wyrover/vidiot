#ifndef STATE_RIGHTDOWN_H
#define STATE_RIGHTDOWN_H

#include <boost/statechart/custom_reaction.hpp>
#include "State.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace state {

class StateRightDown 
    :   public TimeLineState< StateRightDown, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StateRightDown( my_context ctx );

    ~StateRightDown();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvRightUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvRightDown& evt );
    boost::statechart::result react( const EvRightUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvKeyDown& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mPopup;    ///< true if a popup menu is currently being shown.
};

}}} // namespace

#endif // STATE_RIGHTDOWN_H
