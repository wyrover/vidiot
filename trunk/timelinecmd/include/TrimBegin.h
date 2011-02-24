#ifndef TRIM_BEGIN_H
#define TRIM_BEGIN_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class TrimBegin 
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Enlarge/reduce the size of a clip by moving its leftmost position.
    ///
    /// Note that if parameter shift equals true, then all clips (in all tracks)
    /// after the clip's original leftmost pts value are shifted backwards to
    /// accommodate for the extra length. Furthermore note, that clip's leftmost
    /// position (getLeftPts()) will remain at the exact same position in case
    /// shift equals true.
    ///
    /// \param clip clip to be changed
    /// \param diff amount to enlarge/reduce the clip's size by changing its begin point
    /// \param shift indicates if the clip may be enlarged, even when there is no empty space in front of it.
    TrimBegin(gui::timeline::Timeline& timeline, model::ClipPtr clip, pts diff, bool shift);

    ~TrimBegin();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

    //////////////////////////////////////////////////////////////////////////
    // REPORT BACK TO INITIATING CLASS
    //////////////////////////////////////////////////////////////////////////

    ///< \return the new position of the (updated) clip in the track
    pts adjustedPosition() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::ClipPtr mClip;
    pts mDiff;
    bool mShift;
    pts mAdjustedPosition;
};

}}} // namespace

#endif // TRIM_BEGIN_H
