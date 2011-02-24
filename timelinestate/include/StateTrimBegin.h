#ifndef STATE_TRIM_BEGIN_H
#define STATE_TRIM_BEGIN_H

#include <boost/statechart/custom_reaction.hpp>
#include "StateAlways.h"
#include "UtilInt.h"

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
    boost::statechart::result react( const EvKeyUp& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mMustUndo;

    wxPoint mStartPosition;
    wxPoint mCurrentPosition;

    model::ClipPtr mOriginalClip;

    pts mMinDiffClipContent;        ///< Minimum allowed adjustment,  when viewing the contents of the original clip
    pts mMaxDiffClipContent;        ///< Maximum allowed adjustment, when viewing the contents of the original clip
    pts mMinDiffClipSpace;          ///< Minimum allowed adjustment,  when viewing the available free area in front of the original clip
    pts mMinDiffLinkContent;        ///< Minimum allowed adjustment,  when viewing the contents of the linked clip
    pts mMaxDiffLinkContent;        ///< Maximum allowed adjustment, when viewing the contents of the linked clip
    pts mMinDiffLinkSpace;          ///< Minimum allowed adjustment,  when viewing the available free area in front of the linked clip
    pts mMinShiftOtherTrackContent; ///< Minimum allowed shift (to the left) of 'other' tracks
    bool mShiftDown;

    EditDisplay* mEdit;

    pts mOriginalPointerPosition;   ///< Holds the pts position that the mouse pointer is over

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts getLeftEmptyArea(model::ClipPtr clip); ///< /return size of area to the left of clip that is empty

    pts getDiff();
    model::ClipPtr getUpdatedClip();
    void show();
};

}}} // namespace

#endif // STATE_TRIM_BEGIN_H
