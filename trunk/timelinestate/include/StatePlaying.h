#ifndef STATE_PLAYING_H
#define STATE_PLAYING_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"

namespace gui { namespace timeline { namespace state {

class Playing
    :   public TimeLineState< Playing, Always >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Playing( my_context ctx );

    ~Playing();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvKeyUp& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result stop();

    void triggerBegin();
    void triggerEnd();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mMakingNewSelection;
};

}}} // namespace

#endif // STATE_PLAYING_H
