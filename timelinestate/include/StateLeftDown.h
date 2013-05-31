#ifndef STATE_TEST_DRAG_START_H
#define STATE_TEST_DRAG_START_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftDown;
struct EvLeftUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct StateLeftDown
    :   public TimeLineState< StateLeftDown, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StateLeftDown( my_context ctx );

    ~StateLeftDown();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvKeyDown& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

     wxPoint mStartPosition;

     static const wxString sTooltip;
};

}}} // namespace

#endif // STATE_TEST_DRAG_START_H