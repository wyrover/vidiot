#ifndef TRIM_BEGIN_H
#define TRIM_BEGIN_H

#include "AClipEdit.h"
#include "PositionInfo.h"

namespace gui { namespace timeline { namespace command {

class TrimClip
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Enlarge/reduce the size of a clip by moving its leftmost/rightmost position.
    ///
    /// Note that if parameter shift equals true, then all clips (in all tracks)
    /// after the clip's original leftmost pts value are shifted backwards to
    /// accommodate for the extra length. Furthermore note, that clip's leftmost
    /// position (getLeftPts()) will remain at the exact same position in case
    /// shift equals true.
    ///
    /// \param left if true, shift leftmost point of clip. if false, shift rightmost point of clip.
    /// \param clip clip to be changed
    /// \param transition transition that must be unapplied if this is not a shift trim operation (of 0 then nothing needs to be done)
    TrimClip(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, MouseOnClipPosition position, bool left);

    /// \param shift indicates if the clip may be enlarged, even when there is no empty space in front of it.
    /// \param diff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right)
    void update(bool shift, pts diff);

    ~TrimClip();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getClip() const;

    pts getDiff() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mOriginalClip;
    model::IClipPtr mOriginalLink;
    model::IClipPtr mClip;              ///< Same as mOriginalClip or its replacement in case a transition was unapplied
    model::IClipPtr mLink;              ///< Same as mOriginalLink or its replacement in case a transition was unapplied
    // todo mLink and mOriginalLink for the link also, since the link may also be under a transition
    model::TransitionPtr mTransition;
    pts mDiff;
    bool mLeft;
    bool mShift;

    pts mMinShiftOtherTrackContent; ///< Minimum allowed shift (to the left) of 'other' tracks
    pts mMaxShiftOtherTrackContent; ///< Maximum allowed shift (to the right) of 'other' tracks

    MouseOnClipPosition mPosition;  ///< The logical mouse position where the trim was started

    bool mSubmitted;                ///< True if the command has actually been submitted, false if it's only used during the edit process which is aborted

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Determine mMinShiftOtherTrackContent and  mMaxShiftOtherTrackContent. When
    /// shift-trimming all contents in other tracks is shifted accordingly. There must
    /// be room in those tracks to accomodate for that. These two boundaries ensure that
    /// the trim will always be such that, in case of shift trimming, there is enough
    /// room in other tracks.
    void determineShiftBoundariesForOtherTracks();

    // Before the trim is done, the actual diffs must be calculated. This depends
    // one the possible unapplication of transitions. That is already done here.
    void removeTransition();

    // Determine mDiff which is the actual trim to be applied
    // todo rename mDiff to mTrim
    /// \param mousediff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right) given the mouse motion, but not taking into account any boundaries
    void determineTrim(pts mousediff);

    // Apply the actual trim to mClip and mLink
    void applyTrim();

    /// Replace the given clip with a new clip. The new clip's begin and end
    /// are adjusted according to parameters begin and end.
    /// \param clip clip to be replaced
    /// \param begin amount to be removed from beginning of clip
    /// \param end amount to be removed from end of clip
    /// \pre clip is a empty clip
    /// \pre begin >= 0
    /// \pre end >= 0
    void reduceSize(model::IClipPtr emptyclip, pts begin, pts end);

    /// Given the mouse movement, determine how far to stretch/reduce the clip
    /// \param diff mouse movement in video frames length
    pts getDiff(pts diff);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const TrimClip& obj );
};

}}} // namespace

#endif // TRIM_BEGIN_H