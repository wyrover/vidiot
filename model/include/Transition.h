#ifndef MODEL_TRANSITION_H
#define MODEL_TRANSITION_H

#include <wx/event.h>
#include <list>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "IClip.h"

namespace model {

class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef boost::shared_ptr<const Track> ConstTrackPtr;
typedef boost::weak_ptr<Track> WeakTrackPtr;
typedef std::list<TrackPtr> Tracks;
class Transition;
typedef boost::shared_ptr<Transition> TransitionPtr;
typedef std::list<TransitionPtr> Transitions;
typedef boost::shared_ptr<const IClip> ConstIClipPtr;

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
    virtual Transition* clone() override;           
    
    virtual ~Transition();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0) override;
    virtual TrackPtr getTrack() override;

    virtual pts getLeftPts() const override;
    virtual pts getRightPts() const override; 

    virtual void setLink(IClipPtr link) override;
    virtual IClipPtr getLink() const override;

    virtual pts getMinAdjustBegin() const override;
    virtual pts getMaxAdjustBegin() const override;
    virtual void adjustBegin(pts adjustment) override;

    virtual pts getMinAdjustEnd() const override;
    virtual pts getMaxAdjustEnd() const override;
    virtual void adjustEnd(pts adjustment) override;

    virtual bool getSelected() const override;
    virtual void setSelected(bool selected) override;

    virtual bool getDragged() const override;
    virtual void setDragged(bool dragged) override;

    virtual pts getGenerationProgress() const override;          
    virtual void setGenerationProgress(pts progress) override;

    void invalidateLastSetPosition() override;
    boost::optional<pts> getLastSetPosition() const override;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLeft() const;    ///< \return number of frames to the left of the cut between the two clips
    virtual pts getRight() const;   ///< \return number of frames to the right of the cut between the two clips

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
