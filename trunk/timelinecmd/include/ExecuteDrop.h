#ifndef EXECUTE_DROPS_H
#define EXECUTE_DROPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class ExecuteDrop 
    :   public AClipEdit
{
public:

    static const pts sNoShift;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    struct Drop
    {
        model::TrackPtr track;
        pts position;
        model::IClips clips;     ///< Must be contiguous
        friend std::ostream& operator<<( std::ostream& os, const Drop& obj );
    };
    typedef std::list<Drop> Drops;

    ExecuteDrop(model::SequencePtr sequence, model::IClips drags, Drops drops, pts shiftPosition = sNoShift, pts shiftSize = sNoShift);

    ~ExecuteDrop();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::Transitions mTransitions;    ///< List of transitions that must be removed because one but not all of their adjacent clips are moved
    model::IClips mDrags;               ///< List of clips that are removed
    Drops mDrops;
    pts mShiftPosition;
    pts mShiftSize;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// \return true if the transition must be removed when the current drag is dropped
    /// This is required for the purpose of removing a transition of which one but not
    /// all related (left+right) clips are being dragged.
    bool transitionMustBeRemovedOnDrop(model::TransitionPtr transition) const;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ExecuteDrop& obj );
};

}}} // namespace

#endif // EXECUTE_DROPS_H
