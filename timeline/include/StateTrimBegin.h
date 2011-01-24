#ifndef STATE_TRIM_BEGIN_H
#define STATE_TRIM_BEGIN_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"

namespace model {
class Clip;
typedef boost::shared_ptr<Clip> ClipPtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;
struct MoveParameter;
typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
typedef std::list<MoveParameterPtr> MoveParameters; // std::list because moves must be done in a particular order.
}

namespace gui {
    class EditDisplay;
}

namespace gui { namespace timeline { namespace state {

class TrimBegin
    :   public TimeLineState< TrimBegin, Always >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TrimBegin( my_context ctx );

    ~TrimBegin();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvKeyDown& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

     wxPoint mStartPosition;
     wxPoint mCurrentPosition;

     model::ClipPtr mOriginalClip;

     EditDisplay* mEdit;

     //////////////////////////////////////////////////////////////////////////
     // HELPER METHODS
     //////////////////////////////////////////////////////////////////////////

     model::ClipPtr getUpdatedClip();
     void show();
};

}}} // namespace

#endif // STATE_TRIM_BEGIN_H
