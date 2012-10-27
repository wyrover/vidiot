#ifndef MODEL_TRANSITION_H
#define MODEL_TRANSITION_H

#include "IClip.h"

namespace model {

/// Class representing transitions in the timeline. Note that the actual transition
/// is rendered by taking its adjacent clips, cloning these, and adjusting the clones
/// to provide the correct frames.
///
/// When transitions are added to a track, the adjacent clips are shortened in such a
/// way that they 'make room' for the transition. A clips offset is increased and/or
/// its length is reduced.
class Transition
    :   public IClip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ///< Constructor for recovery from disk
    Transition();

      ///< Constructor for creating new transition
    Transition(pts nFramesLeft, pts nFramesRight);

    ///< Used for making deep copies (clones)
    virtual Transition* clone() const override;

    virtual ~Transition();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    pts getLength() const override;
    void moveTo(pts position) override;
    wxString getDescription() const override;
    void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0) override;
    TrackPtr getTrack() override;

    pts getLeftPts() const override;
    pts getRightPts() const override;

    void setLink(IClipPtr link) override;
    IClipPtr getLink() const override;

    pts getMinAdjustBegin() const override;
    pts getMaxAdjustBegin() const override;
    void adjustBegin(pts adjustment) override;

    pts getMinAdjustEnd() const override;
    pts getMaxAdjustEnd() const override;
    void adjustEnd(pts adjustment) override;

    TransitionPtr getInTransition() const override;
    TransitionPtr getOutTransition() const override;

    bool getSelected() const override;
    void setSelected(bool selected) override;

    bool getDragged() const override;
    void setDragged(bool dragged) override;

    pts getGenerationProgress() const override;
    void setGenerationProgress(pts progress) override;

    void invalidateLastSetPosition() override;
    boost::optional<pts> getLastSetPosition() const override;

    std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

    int getIndex() const override;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    pts getTouchPosition() const;   ///< \return position where the two transitioned clips are 'touching'
    pts getLeft() const;    ///< \return number of frames to the left of the cut between the two clips
    pts getRight() const;   ///< \return number of frames to the right of the cut between the two clips

    /// Make the 'in' clip that is to be used for rendering data
    /// This takes the previous clip in the track, clones it, and adjust the
    /// clone such that it has the offset and length as required for the
    /// transition.
    /// If there is no 'in' clip for this transition (out-only transition) then
    /// returns '0'.
    /// \return a clone of the clip to be used for rendering transition data
    model::IClipPtr makeLeftClip() const;

    /// Make the 'out' clip that is to be used for rendering data
    /// This takes the previous clip in the track, clones it, and adjust the
    /// clone such that it has the offset and length as required for the
    /// transition.
    /// If there is no 'out' clip for this transition (in-only transition) then
    /// returns '0'.
    /// \return a clone of the clip to be used for rendering transition data
    model::IClipPtr makeRightClip() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    Transition(const Transition& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mFramesLeft;    ///< Number of frames to the left of the cut between the two clips
    pts mFramesRight;   ///< Number of frames to the right of the cut between the two clips

    boost::optional<pts> mLastSetPosition;  ///< The most recent position as specified in 'moveTo()'.
    pts mGeneratedPts;                      ///< (approximate) pts value of last video/audio returned with getNext*

    WeakTrackPtr mTrack;    ///< Track which holds this transition. Stored as weak_ptr to avoid cyclic dependencies (leading to memory leaks).
    pts mLeftPtsInTrack;    ///< Position inside the track. 0 if not in a track.
    unsigned int mIndex;    ///< Index of this clip in the track (for debugging)
    bool mSelected;         ///< True if this clip is currently selected
    bool mDragged;          ///< True if this clip is currently dragged

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Transition& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Transition, 1)

#endif // MODEL_TRANSITION_H