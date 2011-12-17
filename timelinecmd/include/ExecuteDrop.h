#ifndef EXECUTE_DROPS_H
#define EXECUTE_DROPS_H

#include <set>
#include "AClipEdit.h"
#include "Drag_Shift.h"

namespace gui { namespace timeline { namespace command {

class ExecuteDrop
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TYPES
    //////////////////////////////////////////////////////////////////////////

    typedef std::set<model::IClipPtr> Drags;

    struct Drop
    {
        model::TrackPtr track;
        pts position;
        model::IClips clips;     ///< Must be contiguous
        friend std::ostream& operator<<( std::ostream& os, const Drop& obj );
    };
    typedef std::list<Drop> Drops;

    typedef std::set<model::TransitionPtr> UnappliedTransitions;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ExecuteDrop(model::SequencePtr sequence);

    virtual ~ExecuteDrop();

    /// Called when the drag operation starts. This prepares the timeline for the dragging:
    /// - In-Out-Transitions for which only of of the two adjacent clips are selected, are unapplied
    /// - Transitions for which all 'related' clips are selected, are also made part of the drag
    /// Doing this preparation inside this ClipEdit command ensures that this preparation
    /// (which may make changes to the timeline) will be part of the same command that
    /// does the drag and drop. That ensures that both are Undone with one operation.
    /// Furthermore, explicitly changing the timeline (as opposed to 'faking it') makes
    /// the code handling the drop easier (don't have to take into account that some of
    /// the clips have been changed for unapplying transitions).
    /// \param drags initial list of dragged clips (note that this also applies to new clips being dragged into the timeline)
    /// \param isInsideDrag true if this is a drag and drop inside the timeline. False if new clips are being inserted.
    /// \post mDrags contains updated list of dragged clips
    void onDrag(const Drags& drags, bool isInsideDrag);

    /// Called when the drop operation was finished. After this call, the command will be
    /// executed on the sequence via 'initialize'.
    void onDrop(Drops drops, Shift shift = boost::none);

    /// Called when the drag operation is aborted. Any changes made to the timeline must be
    /// undone. The object will be destructed after this call.
    /// \see onStartDrag for operations that must be undone.
    void onAbort();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// The list of dragged clips is stored in one and only one place to
    /// avoid duplication. Since the updating of these clips involves changing
    /// the timeline (at the start of the drag, some transitions may be
    /// unapplied) it is stored inside this command. That ensures that both
    /// the actual drag and drop as well as its preparation actions are
    /// undone with one undo action.
    const Drags& getDrags() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Drags mDrags; ///< Clips that are removed. Use set to avoid duplicate entries (duplicate entries cause errors since a clip's attributes are changed - removed from a track, for instance - and then the clip is removed 'again' from the now nonexistent track)
    Drops mDrops;
    Shift mShift;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ExecuteDrop& obj );
};

}}} // namespace

#endif // EXECUTE_DROPS_H