#ifndef STATE_MOVE_TRACK_DIVIDER_H
#define STATE_MOVE_TRACK_DIVIDER_H

#include <boost/statechart/custom_reaction.hpp>
#include "State.h"

namespace model { 
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
}

namespace gui { namespace timeline { namespace state {

class MoveTrackDivider
    :   public TimeLineState< MoveTrackDivider, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    MoveTrackDivider( my_context ctx );

    ~MoveTrackDivider();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >,
        boost::statechart::custom_reaction< EvLeave >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvKeyUp& evt);
    boost::statechart::result react( const EvLeave& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxPoint mStartPosition; ///< Original pointer position when starting the resize
    int mOriginalHeight;    ///< Original height of the track
    model::TrackPtr mTrack; ///< The track which is resized

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result abort();
};

}}} // namespace

#endif // STATE_MOVE_TRACK_DIVIDER_H
