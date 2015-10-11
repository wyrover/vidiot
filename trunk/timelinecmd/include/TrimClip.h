// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#pragma once

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
    TrimClip(const model::SequencePtr& sequence, const model::IClipPtr& clip, const model::TransitionPtr& transition, const MouseOnClipPosition& position);

    /// To be called when the trim value is changed (for instance, when mouse is moved during trimming, or when a 'length' button is pressed in the 'clip details' view).
    /// \param diff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right)
    /// \param shift if true, then a shift trim is enforced. Otherwise, the current keyboard (shift) state is used.
    /// \param trimline if true, then the selected clip's linked clip (if any) is trimmed also.
    void update(pts diff, bool shift, bool trimlink);

    /// Set the cursor position to be applied after the trim has been executed.
    /// \param position new cursor position after trimming.
    void setCursorPositionAfter(pts position);

    virtual ~TrimClip();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;
    void doExtraAfter() override;
    void undoExtraAfter() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getOriginalClip() const;
    model::IClipPtr getOriginalLink() const;
    model::IClipPtr getNewClip() const;
    model::IClipPtr getNewLink() const;

    static bool isBeginTrim(const MouseOnClipPosition& position);

    bool isBeginTrim() const;
    bool isShiftTrim() const;

    pts getDiff() const;

    struct TrimLimit
    {
        TrimLimit()
            : Min(std::numeric_limits<pts>::min())
            , Max(std::numeric_limits<pts>::max())
        {
        }
        pts Min;
        pts Max;

        friend std::ostream& operator<<(std::ostream& os, const TrimLimit& obj)
        {
            os << obj.Min << '|' << obj.Max;
            return os;
        }
    };

    /// Determine boundaries (restrictions) on trimming when trimming the given clips
    /// \param sequence for this sequence the boundaries must be determined
    /// \param clip clip to be used for determining the boundaries.
    /// \param link clip that is linked (or going to be linked) to the given clip
    /// \param position logical position (in the clip) where the trim is applied
    /// \param shift if true then determine the boundaries for a shift trim operation
    /// \return minimum and maximum allowed trim values
    /// \pre clip must be part of a track
    /// \pre link == 0 OR link must be part of a track
    /// \note that link must be specified specifically, since in certain scenario's (trimming via the timeline, in case transition removal is involved) the clip has already been replaced (and is not yet coupled to a link).
    static TrimLimit determineBoundaries(const model::SequencePtr& sequence, const model::IClipPtr& clip, const model::IClipPtr& link, const MouseOnClipPosition& position, bool shift);

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
    bool mTrimLink;                         ///< True iff the (trimmed) clip's link must be trimmed also.
    MouseOnClipPosition mPosition;          ///< Logical position where the trim is done
    pts mShiftStart;                        ///< Position at which the shift will start
    model::IClipPtr mReplacementClip;
    pts mCursorPositionBefore;              ///< Position of the cursor before the trim is applied
    pts mCursorPositionAfter;               ///< Position of the cursor after the trim is applied

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    // Before the trim is done, the actual diffs must be calculated. This depends
    // one the possible unapplication of transitions. That is already done here.
    void removeTransition();

    // Apply the actual trim to mClip and mLink
    void applyTrim();

    /// Replace the given clip with a new clip. The new clip's begin and end
    /// are adjusted according to parameters begin and end.
    /// \param clip clip to be replaced
    /// \param begin amount to be added (<0) or removed (>0) from beginning of clip
    /// \param end amount to be added (>0) or removed (<0) from end of clip
    /// \pre !clip->isA<Transition>()
    void adjust(const model::IClipPtr& clip, pts begin, pts end);

    /// Given the mouse movement, determine how far to stretch/reduce the clip
    /// \param diff mouse movement in video frames length
    pts getDiff(pts diff);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TrimClip& obj);
};

}}} // namespace
