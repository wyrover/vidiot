#ifndef MODEL_TRANSITION_H
#define MODEL_TRANSITION_H

#include <list>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>
#include "IControl.h"

namespace model {

// FORWARD DECLARATIONS
class Transition;
typedef boost::shared_ptr<Transition> TransitionPtr;
typedef std::list<TransitionPtr> Transitions;

class Transition
    :   public IControl
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
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLeft() const;    ///< \return number of frames to 'snoop' from left clip
    virtual pts getRight() const;   ///< \return number of frames to 'snoop' from right clip

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength();
    virtual void moveTo(pts position);
    virtual wxString getDescription() const;
    virtual void clean();

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    Transition(const Transition& other);

    //////////////////////////////////////////////////////////////////////////
    // CURRENT POSITION HANDLING
    //////////////////////////////////////////////////////////////////////////

    /// This method resets mLastSetPosition. This must be called whenever there
    /// is new playback progress.
    void invalidateLastSetPosition();

    /// Return the most recent position as specified in moveTo(). This is
    /// uninitialized when there was playback progress after the moveTo.
    /// \see invalidateLastSetPosition
    boost::optional<pts> getLastSetPosition() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mFramesLeft;    ///< Number of frames to use from the left clip
    pts mFramesRight;   ///< Number of frames to use from the right clip

    boost::optional<pts> mLastSetPosition;  ///< The most recent position as specified in 'moveTo()'.

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
BOOST_CLASS_EXPORT(model::Transition)
BOOST_CLASS_TRACKING(model::Transition, boost::serialization::track_always)

#endif // MODEL_TRANSITION_H
