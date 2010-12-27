#ifndef TIMELINE_MOVE_CLIPS_H
#define TIMELINE_MOVE_CLIPS_H

#include <set>
#include <list>
#include <map>
#include "TimelineCommand.h"
#include "ModelPtr.h"
#include "Track.h"

namespace command {

class TimelineMoveClips 
    :   public TimelineCommand
{
public:
    typedef std::map<model::ClipPtr, model::Clips> ReplacementMap;

    /// Constructor only used for deletion of clips yet
    /// @todo make separate command!
    TimelineMoveClips(gui::timeline::Timeline& timeline, model::MoveParameters params);

    struct PasteInfo
    {
        model::TrackPtr track;
        pts position;
        model::Clips clips; ///< Must be contiguous

        friend std::ostream& operator<<( std::ostream& os, const PasteInfo& obj );
    };
    typedef std::list<PasteInfo> Drops;

    /// Drag and drop constructor
    TimelineMoveClips(gui::timeline::Timeline& timeline, model::Clips drags, Drops drops);


    ~TimelineMoveClips();
    bool Do();
    bool Undo();
private:
    model::MoveParameters mParams;
    model::MoveParameters mParamsUndo;

    model::Clips mDrags;
    Drops mDrops;


    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Split the clip at the given (track) position. If there already is a cut at the given
    /// position, then nothing is changed. When the clip is replaced by (two) other clips, then
    /// In the ReplacementMap (if != 0) the mapping clip->(first,second) is added.
    /// @param track track in which a cut must be made at the given position
    /// @param position position in the track where the split should occur
    /// @param conversionmap mapping for 'maintaining links' that will be updated when splitting
    void split(model::TrackPtr track, pts position, ReplacementMap* conversionmap = 0);

    /// Replace the given clip (original) with a list of other clips.
    /// In the ReplacementMap (if != 0) the mapping clip->replacements is added.
    /// @param clip original clip to be replaced
    /// @param replacements clips to be inserted in place of 'clip'
    /// @param conversionmap mapping for 'maintaining links' that will be updated when splitting
    void replaceClip(model::ClipPtr original, model::Clips replacements, ReplacementMap* conversionmap = 0);

    /// Repair 'linking of clips' information after replacing several clips.
    /// For each
    /// Clip c1 - linked to clip l1 - which is replaced with r1,r2,...,rn
    /// and
    /// Clip l1 - linked to clip c1 - which is replaced with s1,s2,...,sn
    /// New links are made as follows:
    /// r1<->s1, r2<->s2, ..., rn<->sn
    ///
    /// If l1 was not replaced yet, it is being replaced with a new clip r1 which is a 100% clone of l1.
    /// If one of the two replacements list is shorter than the other, the extra clips in the other list
    /// are linked to nothing.
    /// @param conversionmap mapping for 'maintaining links' that will be updated when splitting
    void replaceLinks(ReplacementMap& conversionmap);

    /// Add a new Move to the list of moves. Add an inverted Move the list of Undo Moves.
    /// The new Move is executed immediately.
    void newMove(model::TrackPtr addTrack, model::ClipPtr addPosition, model::Clips addClips, model::TrackPtr removeTrack, model::ClipPtr removePosition, model::Clips removeClips);

    /// Execute a move.
    void doMove(model::MoveParameterPtr move);
};

} // namespace

#endif // TIMELINE_MOVE_CLIPS_H
