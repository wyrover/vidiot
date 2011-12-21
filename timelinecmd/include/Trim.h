#ifndef TRIM_BEGIN_H
#define TRIM_BEGIN_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class Trim
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
    /// \param clip clip to be changed
    /// \param transition transition that must be unapplied if this is not a shift trim operation (of 0 then nothing needs to be done)
    /// \param diff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right)
    /// \param left if true, shift leftmost point of clip. if false, shift rightmost point of clip.
    /// \param shift indicates if the clip may be enlarged, even when there is no empty space in front of it.
    Trim(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, pts diff, bool left, bool shift);

    ~Trim();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip;
    model::TransitionPtr mTransition;
    pts mDiff;
    bool mLeft;
    bool mShift;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Replace the given clip with a new clip. The new clip's begin and end
    /// are adjusted according to parameters begin and end.
    /// \param clip clip to be replaced
    /// \param begin amount to be removed from beginning of clip
    /// \param end amount to be removed from end of clip
    /// \pre clip is a empty clip
    /// \pre begin >= 0
    /// \pre end >= 0
    void reduceSize(model::IClipPtr emptyclip, pts begin, pts end);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Trim& obj );
};

}}} // namespace

#endif // TRIM_BEGIN_H