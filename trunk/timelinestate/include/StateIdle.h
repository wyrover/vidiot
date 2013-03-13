#ifndef STATE_IDLE_H
#define STATE_IDLE_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftDown;
struct EvLeftDouble;
struct EvRightDown;
struct EvMotion;
struct EvKeyDown;
struct EvDragEnter;
struct EvPlaybackChanged;

class Idle
    :   public TimeLineState< Idle, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Idle( my_context ctx );

    ~Idle();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvLeftDouble >,
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvDragEnter >,
        boost::statechart::custom_reaction< EvPlaybackChanged >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvLeftDouble& evt );
    boost::statechart::result react( const EvRightDown& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvDragEnter& evt);
    boost::statechart::result react( const EvPlaybackChanged& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result start();

    boost::statechart::result leftDown();

    void addTransition();
};

}}} // namespace

#endif // STATE_IDLE_H