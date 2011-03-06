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

    virtual ~TrimBegin();

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
protected:
    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mTrimBegin;                ///< True if begin of clip is changed, false if end of clip is changed.

    bool mMustUndo;                 ///< True if, for a new update, first a previous trim must be undone

    wxPoint mStartPosition;         ///< Mouse position (in unscrolled coordinates) when the trimming was started
    wxPoint mCurrentPosition;       ///< Current mouse position (in unscrolled coordinates)

    model::ClipPtr mOriginalClip;

    pts mMinShiftOtherTrackContent; ///< Minimum allowed shift (to the left) of 'other' tracks
    pts mMaxShiftOtherTrackContent; ///< Maximum allowed shift (to the right) of 'other' tracks
    bool mShiftDown;                ///< True if shift is down, false if not

    EditDisplay* mEdit;

    pts mFixedPts;                  ///< Pts value (in the track) that must be kept at a fixed pixel position. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.
    pixel mFixedPixel;              ///< Pixel value (physical) that mFixedPts must be aligned with. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts getLeftEmptyArea(model::ClipPtr clip); ///< /return size of area to the left of clip that is empty
    pts getRightEmptyArea(model::ClipPtr clip); ///< /return size of area to the right of clip that is empty

    pts getDiff();
    void preview();
    void show();
};

}}} // namespace

#endif // STATE_TRIM_BEGIN_H
