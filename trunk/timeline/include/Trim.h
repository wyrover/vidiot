#ifndef TRIM_H
#define TRIM_H

#include <list>
#include "Part.h"
#include "PositionInfo.h"
#include "UtilInt.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class Transition;
typedef boost::shared_ptr<Transition> TransitionPtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;
}

namespace gui {
    class EditDisplay;
}

namespace gui { namespace timeline {

struct EvLeftUp;
struct EvLeave;
struct EvMotion;
struct EvKeyUp;
struct EvKeyDown;

class Trim
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Trim(Timeline* timeline);
    ~Trim();

    //////////////////////////////////////////////////////////////////////////
    // TRIM OPERATION
    //////////////////////////////////////////////////////////////////////////

    /// Start trimming
    void start();

    /// Do an update of the trim operation. Must be called upon relevant
    /// mouse/keyboard changes.
    void update();

    void onShift(bool shift);
    void onMove(wxPoint position);

    /// Abort a pending trim operation. If changes were made, undo them.
    void abort();

    /// Stop any pending trim operation. Do not undo the change that was done.
    void stop();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    MouseOnClipPosition mPosition;  ///< The logical mouse position where the trim was started
    bool mTrimBegin;                ///< True if begin of clip is changed, false if end of clip is changed.

    bool mMustUndo;                 ///< True if, for a new update, first a previous trim must be undone

    wxPoint mStartPosition;         ///< Mouse position (in unscrolled coordinates) when the trimming was started
    wxPoint mCurrentPosition;       ///< Current mouse position (in unscrolled coordinates)

    model::IClipPtr mOriginalClip;  ///< Clip whose size is changed
    model::TransitionPtr mTransition;  ///< Transition that is changed, or in case of trimming a clip 'under' a transition, the transition which may need to be unapplied
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
    bool undo();                    ///< \return true if a command was undone.
};

}} // namespace

#endif // TRIM_H