#ifndef STATE_DRAGGING_H
#define STATE_DRAGGING_H

#include "State.h"
#include "EventPart.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftUp;
struct EvDragDrop;
struct EvMotion;
struct EvDragMove;
struct EvLeave;
struct EvDragEnd;
struct EvKeyDown;
struct EvKeyUp;
struct EvZoomChanged;

class Dragging
    :   public TimeLineState< Dragging, Machine >
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
        boost::statechart::custom_reaction< EvKeyUp >,
        boost::statechart::custom_reaction< EvZoomChanged >
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
    boost::statechart::result react( const EvZoomChanged& evt );
};

}}} // namespace

#endif // STATE_DRAGGING_H