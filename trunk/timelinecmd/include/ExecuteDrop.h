#ifndef EXECUTE_DROPS_H
#define EXECUTE_DROPS_H

#include <set>
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

    ExecuteDrop(model::SequencePtr sequence, std::set<model::IClipPtr> drags, Drops drops, pts shiftPosition = sNoShift, pts shiftSize = sNoShift);

    ~ExecuteDrop();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::Transitions mTransitions;    ///< List of transitions that must be removed because one but not all of their adjacent clips are moved
    std::set<model::IClipPtr> mDrags;   ///< Clips that are removed. Use set to avoid duplicate entries (duplicate entries cause errors since a clip's attributes are changed - removed from a track, for instance - and then the clip is removed 'again' from the now nonexistent track)
    Drops mDrops;
    pts mShiftPosition;
    pts mShiftSize;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// \return true if the transition must be removed (unapplied) when the current drag is dropped
    /// This is required for the purpose of removing a transition (and putting the
    /// original clip's length back) of which one but not all related (left+right)
    /// clips are being dragged.
    bool transitionMustBeUnapplied(model::TransitionPtr transition) const;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ExecuteDrop& obj );
};

}}} // namespace

#endif // EXECUTE_DROPS_H
