#ifndef STATE_DRAGGING_H
#define STATE_DRAGGING_H

#include "State.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "Track.h"
#include "GuiPtr.h"
#include "State.h"

namespace gui { namespace timeline { namespace state {

class Dragging 
    :   public boost::statechart::state< Dragging, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Dragging( my_context ctx );

    ~Dragging();

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

    void showDropArea(wxPoint p);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLineClipPtr mClip;

};

}}} // namespace

#endif // STATE_DRAGGING_H
