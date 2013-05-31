#ifndef STATE_MOVE_TRACK_DIVIDER_H
#define STATE_MOVE_TRACK_DIVIDER_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftDown;
struct EvLeftUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct MoveTrackDivider
    :   public TimeLineState< MoveTrackDivider, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    MoveTrackDivider( my_context ctx );

    ~MoveTrackDivider();

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
    boost::statechart::result react( const EvLeave& evt);
    boost::statechart::result react( const EvKeyDown& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mOriginalHeight;    ///< Original height of the track
    model::TrackPtr mTrack; ///< The track which is resized

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result abort();

    static const wxString sTooltip;
};

}}} // namespace

#endif // STATE_MOVE_TRACK_DIVIDER_H