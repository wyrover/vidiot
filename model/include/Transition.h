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

    Transition();                                   ///< Constructor for recovery from disk

    Transition(pts nFramesLeft, pts nFramesRight);  ///< Constructor for creating new transition

    virtual Transition* clone();                    ///< Used for making deep copies (clones) 

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

    virtual pts getGenerationProgress() const;          
    virtual void setGenerationProgress(pts progress);

    void invalidateLastSetPosition();
    boost::optional<pts> getLastSetPosition() const;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLeft() const;    ///< \return number of frames to 'snoop' from left clip
    virtual pts getRight() const;   ///< \return number of frames to 'snoop' from right clip

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

    pts mFramesLeft;    ///< Number of frames to use from the left clip
    pts mFramesRight;   ///< Number of frames to use from the right clip

    boost::optional<pts> mLastSetPosition;  ///< The most recent position as specified in 'moveTo()'.
    pts mGeneratedPts;                      ///< (approximate) pts value of last video/audio returned with getNext*

    TrackPtr mTrack;        ///< Track which holds this transition
    pts mLeftPtsInTrack;    ///< Position inside the track. 0 if not in a track.
    unsigned int mIndex;    ///< Index of this clip in the track (for debugging)
    bool mSelected;         ///< True if this clip is currently selected

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
