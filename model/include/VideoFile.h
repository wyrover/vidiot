#ifndef MODEL_VIDEO_FILE_H
#define MODEL_VIDEO_FILE_H

#include <wx/gdicmn.h>
#include "File.h"
#include "IVideo.h"

namespace model {

class VideoFile;
typedef boost::shared_ptr<VideoFile> VideoFilePtr;

class VideoFile
    :   public File
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoFile();

    VideoFile(wxFileName path);

    virtual VideoFile* clone() override;

    virtual ~VideoFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    void moveTo(pts position) override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(wxSize size, bool alpha = true) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize getSize();

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoFile(const VideoFile& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mDecodingVideo;
    pts mPosition;                  ///< Current position of this clip (set via 'moveTo' or changed via 'getNext')
    VideoFramePtr mDeliveredFrame;  ///< The most recently returned frame in getNext
    pts mDeliveredFrameInputPts;    ///< Input pts (thus, before framerate change) of mDeliveredFrame

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void startDecodingVideo();
    void stopDecodingVideo();

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

#endif // MODEL_VIDEO_FILE_H