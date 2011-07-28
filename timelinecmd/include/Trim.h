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
    /// \param diff amount to enlarge/reduce the clip's size (<0 move to the left, >0 move to the right)
    /// \param left if true, shift leftmost point of clip. if false, shift rightmost point of clip.
    /// \param shift indicates if the clip may be enlarged, even when there is no empty space in front of it.
    Trim(model::SequencePtr sequence, model::IClipPtr clip, pts diff, bool left, bool shift);

    ~Trim();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip;
    pts mDiff;
    bool mLeft;
    bool mShift;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void removewhitespace(model::IClipPtr emptyclip, pts toberemoved, ReplacementMap* conversionmap);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Trim& obj );
};

}}} // namespace

#endif // TRIM_BEGIN_H
