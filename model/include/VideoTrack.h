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

	VideoTrack();
	virtual ~VideoTrack();

    virtual void moveTo(boost::int64_t position);
    void addVideoClip(VideoClipPtr clip);
    void removeVideoClip(VideoClipPtr clip);

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);
private:
    int64_t mPts;


    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::VideoTrack, 1)
BOOST_CLASS_EXPORT(model::VideoTrack)

#endif // MODEL_VIDEO_TRACK_H
