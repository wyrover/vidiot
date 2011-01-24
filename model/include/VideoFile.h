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

    virtual VideoFile* clone();

	virtual ~VideoFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    void moveTo(pts position);

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
    VideoFile(const VideoFile& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void startDecodingVideo();
    void stopDecodingVideo();

    bool mDecodingVideo;

    VideoFramePtr mDeliveredFrame;  ///< The most recently returned frame in getNext
    pts mPosition;                  ///< Current position of this clip (set via 'moveTo' or changed via 'getNext') 
    pts mDeliveredFrameInputPts;    ///< Input pts (thus, before framerate change) of mDeliveredFrame

    //////////////////////////////////////////////////////////////////////////
    // FROM FILE
    //////////////////////////////////////////////////////////////////////////

    void flush();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoFile& obj );

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
BOOST_CLASS_VERSION(model::VideoFile, 1)
BOOST_CLASS_EXPORT(model::VideoFile)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(boost::enable_shared_from_this<VideoFile>)

#endif // MODEL_VIDEO_FILE_H
