#ifndef MODEL_VIDEO_TRANSITION_H
#define MODEL_VIDEO_TRANSITION_H

#include "Transition.h"
#include "IVideo.h"
#include <boost/serialization/assume_abstract.hpp>

namespace model {

class VideoTransition
    :   public Transition
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoTransition();

    VideoTransition(IClipPtr left, pts nFramesLeft, IClipPtr right, pts nFramesRight);

    virtual ~VideoTransition();

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoTransition(const VideoTransition& other);

    //////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION OF TRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getVideo(pts position, int requestedWidth, int requestedHeight, bool alpha) = 0;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mPosition;      ///< Last rendered position

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoTransition& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::VideoTransition, 1);


#endif // MODEL_VIDEO_TRANSITION_H
