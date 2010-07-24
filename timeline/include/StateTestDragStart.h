#ifndef STATE_TEST_DRAG_START_H
#define STATE_TEST_DRAG_START_H

#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "Track.h"
#include "State.h"

namespace gui { namespace timeline { namespace state {

class TestDragStart
    :   public boost::statechart::state< TestDragStart, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TestDragStart( my_context ctx );

    ~TestDragStart();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result start();

    void deleteSelectedClips(model::MoveParameters& moves, GuiTimeLineTracks tracks);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

};

}}} // namespace

#endif // STATE_TEST_DRAG_START_H
