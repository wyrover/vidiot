#ifndef MODEL_VIDEO_TRANSITION_CLIP_H
#define MODEL_VIDEO_TRANSITION_CLIP_H

#include "VideoClip.h"

namespace model {

class VideoTransition;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;

class VideoTransitionClip
    :   public VideoClip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoTransitionClip();

    VideoTransitionClip(VideoTransitionPtr transition);

    virtual VideoTransitionClip* clone();

    virtual ~VideoTransitionClip();

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoTransitionClip(const VideoTransitionClip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoTransitionClip& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::VideoTransitionClip, 1)
BOOST_CLASS_EXPORT(model::VideoTransitionClip)

#endif // MODEL_VIDEO_TRANSITION_CLIP_H
