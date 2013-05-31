#ifndef STATE_RIGHTDOWN_H
#define STATE_RIGHTDOWN_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvRightDown;
struct EvRightUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct StateRightDown
    :   public TimeLineState< StateRightDown, Machine >
{
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

    static const wxString sTooltip;
};

}}} // namespace

#endif // STATE_RIGHTDOWN_H