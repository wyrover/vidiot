#ifndef STATE_PLAYING_H
#define STATE_PLAYING_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftDown;
struct EvRightDown;
struct EvKeyDown;
struct EvKeyUp;
struct EvPlaybackChanged;

struct Playing
    :   public TimeLineState< Playing, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Playing( my_context ctx );

    ~Playing();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >,
        boost::statechart::custom_reaction< EvPlaybackChanged >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvRightDown& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvKeyUp& evt);
    boost::statechart::result react( const EvPlaybackChanged& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void triggerBegin();
    void triggerEnd();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mMakingNewSelection;
};

}}} // namespace

#endif // STATE_PLAYING_H