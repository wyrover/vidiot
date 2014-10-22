// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef EXECUTE_DROPS_H
#define EXECUTE_DROPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline {
class ShiftParams;
typedef boost::shared_ptr<ShiftParams> Shift;
}}

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

typedef std::set<model::IClipPtr> Drags;
struct Drop
{
    model::TrackPtr track;
    pts position;
    model::IClips clips;     ///< Must be contiguous
    friend std::ostream& operator<<(std::ostream& os, const Drop& obj);
    Drop();
    Drop(const Drop& other);
    virtual ~Drop();
};
typedef std::list<Drop> Drops;

/// Important: Link information is kept between the clips being dropped by not replacing the dropped clips with other clips, but just by 'moving the originals around'
class ExecuteDrop
    :   public AClipEdit
{
public:

    typedef std::set<model::TransitionPtr> UnappliedTransitions;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \param sequence sequence on which the drop must be applied
    /// \param external true if the drag originated from an external source (file system, project view)
    explicit ExecuteDrop(const model::SequencePtr& sequence, bool external);

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
    /// \post mDrags contains updated list of dragged clips
    void onDragStart(const Drags& drags);

    /// Called when the drop operation was finished. After this call, the command will be
    /// executed on the sequence via 'initialize'.
    void onDrop(const Drops& drops, const Shift& shift = Shift());

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

    bool mExternal;
    Drops mDrops;
    Drags mDrags; ///< Clips that are removed. Use set to avoid duplicate entries (duplicate entries cause errors since a clip's attributes are changed - removed from a track, for instance - and then the clip is removed 'again' from the now nonexistent track)
    Shift mShift;

    ExecuteDrop();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const ExecuteDrop& obj);
};

}}} // namespace

#endif
