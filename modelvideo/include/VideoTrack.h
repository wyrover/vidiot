#ifndef MODEL_VIDEO_TRACK_H
#define MODEL_VIDEO_TRACK_H

#include "Track.h"
#include "IVideo.h"

namespace model {

class VideoTrack
    :   public Track
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoTrack();

    virtual VideoTrack* clone() const override;

    virtual ~VideoTrack();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // PLAYBACK
    //////////////////////////////////////////////////////////////////////////

    virtual void moveTo(pts position) override;

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoTrack(const VideoTrack& other);

private:

    pts mPts;

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
BOOST_CLASS_VERSION(model::VideoTrack, 1)

#endif // MODEL_VIDEO_TRACK_H