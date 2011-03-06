#ifndef STATE_TEST_DRAG_START_H
#define STATE_TEST_DRAG_START_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"

namespace model {
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;
struct MoveParameter;
typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
typedef std::list<MoveParameterPtr> MoveParameters; // std::list because moves must be done in a particular order.
}

namespace gui { namespace timeline { namespace state {

class TestDragStart
    :   public TimeLineState< TestDragStart, Always >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TestDragStart( my_context ctx );

    ~TestDragStart();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvKeyDown& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

     wxPoint mStartPosition;
};

}}} // namespace

#endif // STATE_TEST_DRAG_START_H
