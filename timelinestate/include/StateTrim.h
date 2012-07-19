#ifndef STATE_TRIM_H
#define STATE_TRIM_H

#include "State.h"

namespace gui {
    class EditDisplay;
}

namespace gui { namespace timeline { namespace state {

struct EvLeftUp;
struct EvLeave;
struct EvMotion;
struct EvKeyUp;
struct EvKeyDown;

class StateTrim
    :   public TimeLineState< StateTrim, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StateTrim( my_context ctx );

    virtual ~StateTrim();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvKeyDown& evt );
    boost::statechart::result react( const EvKeyUp& evt );

};

}}} // namespace

#endif // STATE_TRIM_H