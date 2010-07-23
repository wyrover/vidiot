#ifndef STATE_MOVING_CURSOR_H
#define STATE_MOVING_CURSOR_H

#include "State.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "Track.h"
#include "GuiPtr.h"
#include "State.h"

namespace gui { namespace timeline { namespace state {

class MovingCursor
    :   public boost::statechart::state< MovingCursor, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    MovingCursor( my_context ctx );

    ~MovingCursor();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvKeyUp& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void triggerToggleStart();
    void triggerToggleEnd();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mToggling;
};

}}} // namespace

#endif // STATE_MOVING_CURSOR_H




