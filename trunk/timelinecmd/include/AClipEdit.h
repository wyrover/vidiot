#ifndef CLIP_EDIT_H
#define CLIP_EDIT_H

#include <map>
#include "ATimelineCommand.h"
#include "ModelPtr.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace command {

/// Base class for all edits of clip lengths/position/etc.
/// It provides a reusable undo/redo mechanism for such edits.
/// Furthermore, it contains several helper methods for making
/// the edits.
class AClipEdit 
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AClipEdit(gui::timeline::Timeline& timeline);
    virtual ~AClipEdit();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do();
    bool Undo();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    /// To be implemented by abstract base class specialization
    /// It's this method that is called to create move objects, the first time
    /// the command is executed.
    virtual void initialize() = 0;

protected:

    //////////////////////////////////////////////////////////////////////////
    // MAPPING FOR MAINTAINING LINKED CLIPS
    //////////////////////////////////////////////////////////////////////////
    
    typedef std::map<model::ClipPtr, model::Clips> ReplacementMap;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS FOR SUBCLASSES
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

    /// Find the list of clips, indicated with the pts'es [left, right). Thus, the left pts is part
    /// of these clips, the right pts is not.
    /// @param track of which the clips are to be removed
    /// @param left leftmost pts (the clip at this pts position is removed)
    /// @param right rightmost pts (the clip at this pts position is NOT removed)
    /// @pre a cut exists at both pts positions 'left' and 'right' (use split() to ensure this).
    /// @return list of clips to be removed and their position (that is, the first non-removed clip after these clips, or a null ptr)
    typedef std::pair<model::Clips, model::ClipPtr> ClipsWithPosition;
    ClipsWithPosition findClips(model::TrackPtr track, pts left, pts right);

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

    /// Merge all consecutive empty clips in any track of the sequence into one clip
    /// @see mergeConsecutiveEmptyClips(model::Tracks tracks)
    void mergeConsecutiveEmptyClips();

    /// Merge any consecutive empty clips into one clip for the contiguous region.
    /// @param tracks list of track (audio/video) to be updated
    void mergeConsecutiveEmptyClips(model::Tracks tracks);

    /// Add a new Move to the list of moves. Add an inverted Move the list of Undo Moves.
    /// The new Move is executed immediately.
    void newMove(
        model::TrackPtr addTrack, 
        model::ClipPtr addPosition, 
        model::Clips addClips, 
        model::TrackPtr removeTrack = model::TrackPtr(), 
        model::ClipPtr removePosition = model::ClipPtr(), 
        model::Clips removeClips = model::Clips());

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;                  ///< True if Do has been executed at least once.
    model::MoveParameters mParams;      ///< Holds the actions to make the 'forward' (Do) change
    model::MoveParameters mParamsUndo;  ///< Holds the actions to make the 'reverse' (Undo) change

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Execute a move.
    void doMove(model::MoveParameterPtr move);
};

}}} // namespace

#endif // CLIP_EDIT_H
