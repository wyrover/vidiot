#ifndef STATE_MOVE_DIVIDER_H
#define STATE_MOVE_DIVIDER_H

#include "State.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct MoveDivider
    :   public TimeLineState< MoveDivider, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    MoveDivider( my_context ctx );

    ~MoveDivider();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt);
    boost::statechart::result react( const EvKeyDown& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pixel mOriginalPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result abort();

    static const wxString sTooltip;
};

}}} // namespace

#endif // STATE_MOVE_DIVIDER_H