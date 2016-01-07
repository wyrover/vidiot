// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "ATimelineCommand.h"

namespace gui { namespace timeline { namespace cmd {

class LinkReplacementMap;

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

    explicit AClipEdit(const model::SequencePtr& sequence);
    virtual ~AClipEdit();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    /// This method can be used by derived commands to do extra actions that
    /// are not related to adding/removing/replacing/changing clips when doing
    /// a certain timeline operation.
    ///
    /// This method is executed before the model changes are made.
    ///
    /// Note: be careful when implementing this method for a command which uses
    ///       Revert(), for instance after showing an animation. Initialize()
    ///       (which calls doExtra) is only called after submitting a command.
    ///       However, Revert() calls Undo() which calls undoExtra(). That can
    ///       cause issues.
    virtual void doExtraBefore();

    /// This method can be used by derived commands to do extra actions that
    /// are not related to adding/removing/replacing/changing clips when doing
    /// a certain timeline operation.
    ///
    /// This method is executed after the model changes are made.
    ///
    /// Note: be careful when implementing this method for a command which uses
    ///       Revert(), for instance after showing an animation. Initialize()
    ///       (which calls doExtra) is only called after submitting a command.
    ///       However, Revert() calls Undo() which calls undoExtra(). That can
    ///       cause issues.
    virtual void doExtraAfter();

    /// This method can be used by derived commands to undo extra actions that
    /// are not related to adding/removing/replacing/changing clips when undoing
    /// a certain timeline operation.
    ///
    /// This method is executed before the model changes are made.
    ///
    /// Note: be careful when implementing this method for a command which uses
    ///       Revert(), for instance after showing an animation. Initialize()
    ///       (which calls doExtra) is only called after submitting a command.
    ///       However, Revert() calls Undo() which calls undoExtra(). That can
    ///       cause issues.
    virtual void undoExtraBefore();

    /// This method can be used by derived commands to undo extra actions that
    /// are not related to adding/removing/replacing/changing clips when undoing
    /// a certain timeline operation.
    ///
    /// This method is executed after the model changes are made.
    ///
    /// Note: be careful when implementing this method for a command which uses
    ///       Revert(), for instance after showing an animation. Initialize()
    ///       (which calls doExtra) is only called after submitting a command.
    ///       However, Revert() calls Undo() which calls undoExtra(). That can
    ///       cause issues.
    virtual void undoExtraAfter();

    /// To be implemented by derived classes.
    /// It's this method that is called to create move objects, the first time
    /// the command is executed.
    virtual void initialize();

    /// \return true if the edit has been initialized
    /// When this method returns true the edit has been initialized. That means
    /// that all 'standard' activities (like, replacing link mappings) have been
    /// executed.
    bool isInitialized();

    /// Undo all changes done so far, and clear the Do/Undo administration
    /// Used for edits that already update the model during the edit operation,
    /// before the command is actually submitted.
    void revert();

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS FOR OTHER CLASSES
    //////////////////////////////////////////////////////////////////////////

    /// Make clones of the given clip and its link (if any).
    /// If the clip is linked, then both the clip and its link are cloned.
    /// In that case, the two clones are linked also.
    /// \param clip clip to be cloned
    /// \return clone of clip, and clone of the link of the clip (if any)
    static std::pair<model::IClipPtr, model::IClipPtr> clone(model::IClipPtr clip);

protected:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS FOR SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    /// Split the clip at the given (track) position. If there already is a cut at the given
    /// position, then nothing is changed. When the clip is replaced by (two) other clips, then
    /// In the ReplacementMap (if != 0) the mapping clip->(first,second) is added.
    /// \param track track in which a cut must be made at the given position
    /// \param position position in the track where the split should occur
    void split(const model::TrackPtr& track, pts position);

    /// Replace the given clip (original) with a list of other clips.
    /// In the ReplacementMap (if != 0) the mapping clip->replacements is added.
    /// \param clip original clip to be replaced
    /// \param replacements clips to be inserted in place of 'clip'
    /// \param maintainlinks if true, then the replacements will be linked to the replacement clips of the links of the original clip (ahum...)
    void replaceClip(const model::IClipPtr& original, const model::IClips& replacements, bool maintainlinks = true);

    /// Add given clip to given track at given position. This method is only allowed
    /// for new clips (clip that are not yet contained in a track). For existing
    // (part of a track) clips, replaceClip must be used.
    /// \param clip clip to be inserted
    /// \param track track into which insertion must be done
    /// \param position insertion position
    /// \pre !clip->getLink()
    void addClip(const model::IClipPtr& clip, const model::TrackPtr& track, const model::IClipPtr& position);

    /// Add given clips to given track at given position. This method is only allowed
    /// for new clips (clip that are not yet contained in a track). For existing
    // (part of a track) clips, replaceClip must be used.
    /// \param clips clips to be inserted
    /// \param track track into which insertion must be done
    /// \param position insertion position (default: at end of track)
    /// \pre FOREACH clip in clips: !clip->getLink()
    void addClips(const model::IClips& clips, const model::TrackPtr& track, const model::IClipPtr& position = model::IClipPtr());

    /// Remove the given clip
    /// \param clip original clip to be removed
    void removeClip(const model::IClipPtr& original);

    /// Remove the given list of clips
    /// \param originals list of clips to be removed
    /// \pre originals is a contiguous list of clips in one track
    void removeClips(const model::IClips& originals);

    /// Find the list of clips, indicated with the pts'es [left, right). Thus, the left pts is part
    /// of these clips, the right pts is not.
    /// \param track of which the clips are to be removed
    /// \param left leftmost pts (the clip at this pts position is removed)
    /// \param right rightmost pts (the clip at this pts position is NOT removed)
    /// \pre a cut exists at both pts positions 'left' and 'right' (use split() to ensure this).
    /// \return list of clips to be removed and their position (that is, the first non-removed clip after these clips, or a null ptr)
    typedef std::pair<model::IClips, model::IClipPtr> ClipsWithPosition;
    static ClipsWithPosition findClips(const model::TrackPtr& track, pts left, pts right);

    /// Move all clips in all tracks a certain amount.
    /// \param start only clips clips that are on or after this position must be moved
    /// \param amount distance that must be shifted
    /// \param exclude list of tracks that are not to be changed
    void shiftAllTracks(pts start, pts amount, const model::Tracks& exclude);

    /// Move all clips in the given tracks a certain amount.
    /// \param tracks list of tracks which must be shifted
    /// \param start only clips clips that are on or after this position must be moved
    /// \param amount distance that must be shifted
    /// \param exclude list of tracks that are not to be changed
    void shiftTracks(const model::Tracks& tracks, pts start, pts amount);

    /// Add a new transition replacing the two given clips with two new clips with the transition in between.
    /// The length of the new clips adjacent to the transition is such that they're shortened with the length of the transition.
    /// \param leftClip clip to the left of the transition. May be 0 in case only right clip applies.
    /// \param rightClip clip to the right of the transition. May be 0 in case only left clip applies.
    model::IClipPtr addTransition(const model::IClipPtr& leftClip, const model::IClipPtr& rightClip, const model::TransitionPtr& transition );

    /// Remove transition. That means remove the transition AND any clip (part) it was
    /// covering, leaving white space as a result.
    /// \param transition transition to be removed
    void removeTransition(const model::TransitionPtr& transition );

    /// Unapply transition. If there are adjacent clips that are part of the transition,
    /// these clips will be extended with the part of that clip that is 'part of the transition'.
    /// \param transition transition to be removed
    /// \param replacelinkedclipsalso if true, then the links of any affected clips are also replaced (with clones)
    /// \return list of clips with which the transition and all its related clips are replaced
    ///
    /// About replacelinkedclipsalso: This is sometimes required beacuse the clip edit itselves does not
    /// make new clips for all clips (example: drag and drop), but the unapplytransition DOES changes some
    /// clips. In that case, it must be ensured that theres also a replacement clip for the link.
    model::IClips unapplyTransition(const model::TransitionPtr& transition, bool replacelinkedclipsalso = false );

    /// Replace the given list of clips with one empty clip of the same length. Note that the given
    /// list of clips must be consecutive clips within one track.
    /// \param clips list of clips to be replaced
    /// \return replacement (empty) clip
    model::IClipPtr replaceWithEmpty(const model::IClips& clips);

    /// Make an animation that shows removing the given empty areas (all remaining clips are
    /// shifted over the empty areas). Note that this is used to animate the edit operation, and
    /// results in a large undo history (each empty clip is replaced by a smaller empty clip, which
    /// in turn is replaced with a smaller empty clip, etc).
    /// \post Timeline transaction is started
    ///
    /// \param emptyareas list of empty clips to be trimmed away (does not need to be a list of consecutive clips in one track)
    ///
    /// In general, use this operation to show the animation in the following way:
    /// - animatedTrimEmpty()
    /// - timeline::endTransaction()
    void animatedDeleteAndTrim(const model::IClips& clipsToBeRemoved);

    /// Split all tracks at the interval's begin and end positions.
    /// \return lists of adjacent clips between interval begin and end positions (for all intervals)
    model::IClips splitTracksAndFindClipsToBeRemoved(const PtsIntervals& removed);

    /// Store the current selection for restoring later
    /// \see restoreSelection
    /// \post mSelection
    void storeSelection();

    /// Restore a previously store selection
    /// \pre mSelection
    void restoreSelection();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;                      ///< True if Do has been executed at least once.
    model::MoveParameters mParams;          ///< Holds the actions to make the 'forward' (Do) change
    model::MoveParameters mParamsUndo;      ///< Holds the actions to make the 'reverse' (Undo) change

    boost::optional< model::IClips > mSelected; ///< Store selection

    boost::shared_ptr<LinkReplacementMap> mReplacements; ///< Holds administration for keeping linked clips linked over edits.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Add a new Move to the list of moves. Add an inverted Move the list of Undo Moves.
    /// The new Move is executed immediately. This method is also used to add new clips,
    /// by using the defaults for the remove* parameters.
    void newMove(
        const model::TrackPtr& addTrack,
        const model::IClipPtr& addPosition,
        const model::IClips& addClips,
        const model::TrackPtr& removeTrack = model::TrackPtr(),
        const model::IClipPtr& removePosition = model::IClipPtr(),
        const model::IClips& removeClips = model::IClips());

    /// Execute a move.
    void doMove(const model::MoveParameterPtr& move);

    /// For all replaced clips, ensure that the linked clip (if any) is also replaced,
    /// at least with just a plain clone of the original link. This is needed to
    /// avoid having these links 'dangling' after removal (for instance, when deleting
    /// only the audio part of a audio-video couple, by moving a large new audio clip
    /// over the audio part of the couple).
    void avoidDanglingLinks();


    /// Merge all consecutive empty clips in any track of the sequence into one clip
    /// \see mergeConsecutiveEmptyClips(model::Tracks tracks)
    void mergeConsecutiveEmptyClips();

    /// Merge any consecutive empty clips into one clip for the contiguous region.
    /// Remove any empty clips of length 0
    /// \param tracks list of track (audio/video) to be updated
    void mergeConsecutiveEmptyClips(const model::Tracks& tracks);

    /// If there's an empty clip at the end of the track it has no purpose other than
    /// enlarging the scrolled area. Thus, remove these last empty clips. That'll
    /// potentially avoid adding a lot of unwanted black video at the end, after
    /// rendering.
    void removeEmptyClipsAtEndOfTracks();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const AClipEdit& obj);
};

}}} // namespace
