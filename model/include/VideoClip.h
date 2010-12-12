#ifndef MODEL_VIDEO_CLIP_H
#define MODEL_VIDEO_CLIP_H

#include "Clip.h"
#include "IVideo.h"

namespace model {

class VideoClip
    :   public Clip
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoClip();

    VideoClip(VideoFilePtr clip);

    virtual VideoClip* clone();

    virtual ~VideoClip();

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// @see make_cloned
    VideoClip(const VideoClip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mProgress; ///< Current render position in pts units (delivered video frames count)

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
BOOST_CLASS_VERSION(model::VideoClip, 1)
BOOST_CLASS_EXPORT(model::VideoClip)

#endif // MODEL_VIDEO_CLIP_H
