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
    /// \param sequence sequence that is adjusted
    /// \param clip clip to be changed
    /// \param transition transition that must be unapplied if this is not a shift trim operation (if 0 then nothing needs to be done)
    /// \parm position position where the trim is done
    TrimClip(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, MouseOnClipPosition position);

    /// To be called when the trim value is changed (for instance, when mouse is moved during trimming, or when a 'length' button is pressed in the 'clip details' view).
    /// \param diff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right)
    /// \param shift if true, then a shift trim is enforced. Otherwise, the current keyboard (shift) state is used.
    /// \param position position when the trim was started.
    /// \note The position may be changed between differnet updates. Not required for 'normal' (mouse operated) trimming operations, but required for changing a clip's length from the 'clip details' view.
    void update(pts diff, bool shift);

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

    static bool isBeginTrim(MouseOnClipPosition position); // todo make obsolete

    bool isBeginTrim() const; // todo make obsolete

    pts getDiff() const;

    pts getShiftStart() const;

    struct TrimLimit
    {
        TrimLimit()
            : Min(std::numeric_limits<pts>::min())
            , Max(std::numeric_limits<pts>::max())
        {
        }
        pts Min;
        pts Max;
    };
    struct TrimLimits
    {
        TrimLimit WithShift;
        TrimLimit WithoutShift;
    };

    /// Determine boundaries (restrictions) on trimming when trimming the given clips
    /// \param sequence for this sequence the boundaries must be determined
    /// \param clip clip to be used for determining the boundaries.
    /// \param link clip that is linked (or going to be linked) to the given clip
    /// \param position logical position (in the clip) where the trim is applied
    /// \param shift if true then determine the boundaries for a shift trim operation
    /// \pre clip must be part of a track
    /// \pre link == 0 OR link must be part of a track
    /// \note that link must be specified specifically, since in certain scenario's (trimming via the timeline, in case transition removal is involved) the clip has already been replaced (and is not yet coupled to a link).
    static TrimLimits determineBoundaries(model::SequencePtr sequence, model::IClipPtr clip, model::IClipPtr link, MouseOnClipPosition position, bool shift);

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
    pts mTrim;                              ///< Actual trim adjustment that is applied to mClip
    bool mShift;                            ///< True iff clips in other tracks must be shifted along with the trim operation
    MouseOnClipPosition mPosition;          ///< Logical position where the trim is done
    pts mShiftStart;                        ///< Position at which the shift will start
    model::IClipPtr mReplacementClip;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

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