#ifndef TRIM_CLIP_H
#define TRIM_CLIP_H

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
    /// Note that if shift equals true, then all clips (in all tracks)
    /// after the clip's original leftmost pts value are shifted backwards to
    /// accommodate for the extra length. Furthermore note, that clip's leftmost
    /// position (getLeftPts()) will remain at the exact same position in case
    /// shift equals true.
    ///
    /// \param left if true, shift leftmost point of clip. if false, shift rightmost point of clip.
    /// \param clip clip to be changed
    /// \param transition transition that must be unapplied if this is not a shift trim operation (if 0 then nothing needs to be done)
    /// \param position position when the trim was started
    TrimClip(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, MouseOnClipPosition position);

    /// To be called when the mouse is moved
    /// \param diff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right)
    /// \param shift if true, then a shift trim is enforced.
    void update(pts diff, bool shift = false);

    ~TrimClip();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getOriginalClip() const;
    model::IClipPtr getOriginalLink() const;
    model::IClipPtr getNewClip() const;
    model::IClipPtr getNewLink() const;

    bool isBeginTrim() const;

    pts getDiff() const;

    pts getShiftStart() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mOriginalClip;
    model::TransitionPtr mTransition;       ///< The (optional) transition before or after mOriginalClip, or the transition which IS mClip
    model::IClipPtr mOriginalLink;
    model::TransitionPtr mLinkTransition;   ///< The (optional) transition before or after mOriginalLink, or the transition which IS mClip
    model::IClipPtr mClip;                  ///< Same as mOriginalClip or its replacement in case a transition was unapplied
    model::IClipPtr mLink;                  ///< Same as mOriginalLink or its replacement in case a transition was unapplied
    model::IClipPtr mNewClip;               ///< Clip that replaces the original clip after the trim. Maybe be '0' if the clip was fully trimmed away.
    model::IClipPtr mNewLink;               ///< Clip that replaces the original link after the trim. Maybe be '0' if the clip was fully trimmed away.
    bool mClipIsPartOfTransition;           ///< True if mClip is part of a transition (either before or after it). Used to avoid deleting the clip completely (which is prohibited, since a part of the clip must remain for the transition).
    bool mLinkIsPartOfTransition;           ///< True if mLink is part of a transition (either before or after it). Used to avoid deleting the clip completely (which is prohibited, since a part of the clip must remain for the transition).
    pts mTrim;
    bool mShift;
    pts mMinShiftOtherTrackContent;         ///< Minimum allowed shift (to the left) of 'other' tracks
    pts mMaxShiftOtherTrackContent;         ///< Maximum allowed shift (to the right) of 'other' tracks
    MouseOnClipPosition mPosition;          ///< The logical mouse position where the trim was started
    pts mShiftStart;                        ///< Position at which the shift will start
    model::IClipPtr mReplacementClip;

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
    /// \param mousediff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right) given the mouse motion, but not taking into account any boundaries
    void determineTrim(pts mousediff);

    // Apply the actual trim to mClip and mLink
    void applyTrim();

    /// Replace the given clip with a new clip. The new clip's begin and end
    /// are adjusted according to parameters begin and end.
    /// \param clip clip to be replaced
    /// \param begin amount to be added (<0) or removed (>0) from beginning of clip
    /// \param end amount to be added (>0) or removed (<0) from end of clip
    /// \pre !clip->isA<Transition>()
    void adjust(model::IClipPtr clip, pts begin, pts end);

    /// Given the mouse movement, determine how far to stretch/reduce the clip
    /// \param diff mouse movement in video frames length
    pts getDiff(pts diff);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const TrimClip& obj );
};
}}} // namespace

#endif // TRIM_CLIP_H