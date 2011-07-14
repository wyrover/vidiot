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
typedef std::list<TrackPtr> Tracks;
class Transition;
typedef boost::shared_ptr<Transition> TransitionPtr;
typedef std::list<TransitionPtr> Transitions;

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
    Transition(IClipPtr left, pts nFramesLeft, IClipPtr right, pts nFramesRight);

    ///< Used for making deep copies (clones) 
    virtual Transition* clone();           
    
    virtual ~Transition();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength();
    virtual void moveTo(pts position);
    virtual wxString getDescription() const;
    virtual void clean();

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0);
    virtual TrackPtr getTrack();
    virtual pts getLeftPts() const;
    virtual pts getRightPts() const; 

    virtual void setLink(IClipPtr link);
    virtual IClipPtr getLink() const;

    virtual pts getMinAdjustBegin() const;
    virtual pts getMaxAdjustBegin() const;
    virtual void adjustBegin(pts adjustment);

    virtual pts getMinAdjustEnd() const;
    virtual pts getMaxAdjustEnd() const;
    virtual void adjustEnd(pts adjustment);

    virtual bool getSelected() const;
    virtual void setSelected(bool selected);

    virtual bool getDragged() const;
    virtual void setDragged(bool dragged);

    virtual pts getGenerationProgress() const;          
    virtual void setGenerationProgress(pts progress);

    void invalidateLastSetPosition();
    boost::optional<pts> getLastSetPosition() const;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLeft() const;    ///< \return number of frames to the left of the cut between the two clips
    virtual pts getRight() const;   ///< \return number of frames to the right of the cut between the two clips

    virtual IClipPtr getLeftClip() const;   ///< \return clip to the left of this transition
    virtual IClipPtr getRightClip() const;  ///< \return clip to the right of this transition

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

    IClipPtr mLeft;     ///< Clip to the left of transition
    IClipPtr mRight;    ///< Clip to the right of transition

    pts mFramesLeft;    ///< Number of frames to the left of the cut between the two clips
    pts mFramesRight;   ///< Number of frames to the right of the cut between the two clips

    boost::optional<pts> mLastSetPosition;  ///< The most recent position as specified in 'moveTo()'.
    pts mGeneratedPts;                      ///< (approximate) pts value of last video/audio returned with getNext*

    TrackPtr mTrack;        ///< Track which holds this transition
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
//BOOST_CLASS_TRACKING(model::Transition, boost::serialization::track_always)

#endif // MODEL_TRANSITION_H
