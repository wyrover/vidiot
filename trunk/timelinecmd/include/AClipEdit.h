#ifndef CLIP_EDIT_H
#define CLIP_EDIT_H

#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include "ATimelineCommand.h"
#include "UtilInt.h"

namespace model {
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;

class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef boost::weak_ptr<IClip> WeakIClipPtr;
typedef std::list<IClipPtr> IClips;

class Transition;
typedef boost::shared_ptr<Transition> TransitionPtr;
typedef std::list<TransitionPtr> Transitions;

struct MoveParameter;
typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
typedef std::list<MoveParameterPtr> MoveParameters;
}

namespace gui { namespace timeline { namespace command {

typedef std::map<model::IClipPtr, model::IClips> ReplacementMap;

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

    explicit AClipEdit(model::SequencePtr sequence);
    virtual ~AClipEdit();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    /// To be implemented by abstract base class specialization
    /// It's this method that is called to create move objects, the first time
    /// the command is executed.
    virtual void initialize() = 0;

protected:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS FOR SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    /// Split the clip at the given (track) position. If there already is a cut at the given
    /// position, then nothing is changed. When the clip is replaced by (two) other clips, then
    /// In the ReplacementMap (if != 0) the mapping clip->(first,second) is added.
    /// \param track track in which a cut must be made at the given position
    /// \param position position in the track where the split should occur
    void split(model::TrackPtr track, pts position);

    /// Replace the given clip (original) with a list of other clips.
    /// In the ReplacementMap (if != 0) the mapping clip->replacements is added.
    /// \param clip original clip to be replaced
    /// \param replacements clips to be inserted in place of 'clip'
    void replaceClip(model::IClipPtr original, model::IClips replacements);

    /// \see replaceClip()
    /// This replaces multiple clips, one by one replacing a clip in list
    /// 'originals' with a clip in list replacements. If the list of
    /// replacements is larger, the last clip in originals is replaced with
    /// the remainder replacement clips. If the list of replacements is shorter,
    /// the remainder original clips are replaced with nothing.
    /// \pre originals.size() > 1 || replacements.size() > 1
    void replaceClips(model::IClips originals, model::IClips replacements);

    /// Add given clip to given track at given position. This method is only allowed 
    /// for new clips (clip that are not yet contained in a track). For existing
    // (part of a track) clips, replaceClip must be used.
    /// \pre !clip->getLink()
    void addClip(model::IClipPtr clip, model::TrackPtr track, model::IClipPtr position);

    /// Remove the given clip
    /// \param clip original clip to be removed
    void removeClip(model::IClipPtr original);

    /// Find the list of clips, indicated with the pts'es [left, right). Thus, the left pts is part
    /// of these clips, the right pts is not.
    /// \param track of which the clips are to be removed
    /// \param left leftmost pts (the clip at this pts position is removed)
    /// \param right rightmost pts (the clip at this pts position is NOT removed)
    /// \pre a cut exists at both pts positions 'left' and 'right' (use split() to ensure this).
    /// \return list of clips to be removed and their position (that is, the first non-removed clip after these clips, or a null ptr)
    typedef std::pair<model::IClips, model::IClipPtr> ClipsWithPosition;
    ClipsWithPosition findClips(model::TrackPtr track, pts left, pts right);

    /// Move all clips in all tracks a certain amount. 
    /// \param start only clips clips that are on or after this position must be moved
    /// \param amount distance that must be shifted
    /// \param exclude list of tracks that are not to be changed
    void shiftAllTracks(pts start, pts amount, model::Tracks exclude);

    /// Move all clips in the given tracks a certain amount. 
    /// \param tracks list of tracks which must be shifted
    /// \param start only clips clips that are on or after this position must be moved
    /// \param amount distance that must be shifted
    /// \param exclude list of tracks that are not to be changed
    void shiftTracks(model::Tracks tracks, pts start, pts amount);

    /// Make a new transition replacing the two given clips with two new clips
    /// with a transition in between.
    /// \param leftClip clip to the left of the transition. May be 0 in case only right clip applies.
    /// \param leftLength length to the left of the cut between the clips to be used for the transition
    /// \param rightClip clip to the right of the transition. May be 0 in case only left clip applies.
    /// \param rightLength length to the right of the cut between the clips to be used for the transition
    model::IClipPtr makeTransition( model::IClipPtr leftClip, pts leftLength, model::IClipPtr rightClip, pts rightLength );

    /// Remove transition. That means remove the transition AND any clip (part) it was
    /// covering, leaving white space as a result.
    /// \param transition transition to be removed
    void removeTransition( model::TransitionPtr transition );

    /// Unapply transition. If there are adjacent clips that are part of the transition,
    /// these clips will be extended with the part of that clip that is 'part of the transition'.
    /// \param transition transition to be removed
    void unapplyTransition( model::TransitionPtr transition );

    /// Determine if the given clip has already been removed/replaced in 
    /// a previous step.
    /// \return true if the clip has already been removed/replaced
    bool hasBeenReplaced(model::IClipPtr clip) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;                  ///< True if Do has been executed at least once.
    model::MoveParameters mParams;      ///< Holds the actions to make the 'forward' (Do) change
    model::MoveParameters mParamsUndo;  ///< Holds the actions to make the 'reverse' (Undo) change

    /// Holds all replacements that were done for this command.
    /// Used to keep updated clip link information correct.
    /// Note that a clip may be mapped onto an empty list. That indicates
    /// the clip has been removed, without replacing it with new clips.
    ReplacementMap mReplacements;       

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Add a new Move to the list of moves. Add an inverted Move the list of Undo Moves.
    /// The new Move is executed immediately. This method is also used to add new clips, 
    /// by using the defaults for the remove* parameters.
    /// This is private for a reason: All the public methods use the 'conversionmap'.
    /// This ensures that all clip changes are done while updating the conversionmap.
    ///
    /// Rationale:
    /// Changing clips (particularly, removing them completely) but not administering
    /// them in the conversionmap can cause problems. Example:
    /// - Clip V1 is linked to clip A1
    /// - Clip V1 is replaced with clip VR1
    /// - Clip A1 is removed completely
    /// If. at the end of the edit, the link replacement is done (replaceLinks), then
    /// it's unclear to what clip(s) VR1 must be linked (there is no information 
    /// indicating that A1 was actually removed). Therefore: all edits must use 
    /// the link conversion map.
    void newMove(
        model::TrackPtr addTrack, 
        model::IClipPtr addPosition, 
        model::IClips addClips, 
        model::TrackPtr removeTrack = model::TrackPtr(), 
        model::IClipPtr removePosition = model::IClipPtr(), 
        model::IClips removeClips = model::IClips());

    /// Execute a move.
    void doMove(model::MoveParameterPtr move);

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
    /// \param conversionmap mapping for 'maintaining links' that will be updated when splitting
    void replaceLinks();

    /// Replace the given list of clips with one empty clip of the same length. Note that the given
    /// list of clips must be consecutive clips within one track.
    /// \param clips list of clips to be replaced
    void replaceWithEmpty(model::IClips clips);

    /// Merge all consecutive empty clips in any track of the sequence into one clip
    /// \see mergeConsecutiveEmptyClips(model::Tracks tracks)
    void mergeConsecutiveEmptyClips();

    /// Merge any consecutive empty clips into one clip for the contiguous region.
    /// \param tracks list of track (audio/video) to be updated
    void mergeConsecutiveEmptyClips(model::Tracks tracks);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AClipEdit& obj );
};

}}} // namespace

#endif // CLIP_EDIT_H
