#ifndef STATE_DRAGGING_H
#define STATE_DRAGGING_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"

namespace gui { namespace timeline { namespace state {

class Dragging 
    :   public TimeLineState< Dragging, StateTop::orthogonal<0> >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Dragging( my_context ctx );

    ~Dragging();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvDragDrop >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvDragMove >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvDragEnd >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvDragDrop& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvDragMove& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvDragEnd& evt );
    boost::statechart::result react( const EvKeyDown& evt );
    boost::statechart::result react( const EvKeyUp& evt );
};

}}} // namespace

#endif // STATE_DRAGGING_H
