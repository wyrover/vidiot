#ifndef STATE_TRIM_H
#define STATE_TRIM_H

#include <boost/statechart/custom_reaction.hpp>
#include "UtilInt.h"
#include "State.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
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

struct EvLeftUp;
struct EvLeave;
struct EvMotion;
struct EvKeyUp;
struct EvKeyDown;

class Trim
    :   public TimeLineState< Trim, Machine >
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Trim( my_context ctx );

    virtual ~Trim();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
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

    model::IClipPtr mOriginalClip;
    boost::shared_ptr<wxBitmap> mAdjacentBitmap;

    pts mMinShiftOtherTrackContent; ///< Minimum allowed shift (to the left) of 'other' tracks
    pts mMaxShiftOtherTrackContent; ///< Maximum allowed shift (to the right) of 'other' tracks
    bool mShiftDown;                ///< True if shift is down, false if not

    EditDisplay* mEdit;

    pts mFixedPts;                  ///< Pts value (in the track) that must be kept at a fixed pixel position. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.
    pixel mFixedPixel;              ///< Pixel value (physical) that mFixedPts must be aligned with. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts getDiff();
    void preview();
    void show();
    bool undo();                    ///< \return true if a command was undone.
};

}}} // namespace

#endif // STATE_TRIM_H
