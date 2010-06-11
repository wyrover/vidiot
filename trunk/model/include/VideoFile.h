#ifndef MODEL_VIDEO_FILE_H
#define MODEL_VIDEO_FILE_H

#include "File.h"
#include "IVideo.h"

namespace model {

class VideoFile 
    :   public File
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////


    VideoFile();

    VideoFile(boost::filesystem::path path);

	virtual ~VideoFile();

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

private:

    void startDecodingVideo();
    void stopDecodingVideo();

    bool mDecodingVideo;

    double mVideoAspectRatio;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::VideoFile, 1)
BOOST_CLASS_EXPORT(model::VideoFile)

#endif // MODEL_VIDEO_FILE_H
