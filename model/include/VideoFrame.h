#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include <wx/bitmap.h>
#include <boost/shared_ptr.hpp>
#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilEnum.h"
#include "UtilFrameRate.h"
#include "UtilRTTI.h"

struct AVFrame;

namespace model {

DECLAREENUM(VideoFrameType, \
    videoRGB, \
    videoRGBA);

typedef boost::uint8_t** DataPointer;
typedef int* LineSizePointer;
class VideoFrame;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef std::list<VideoFramePtr> VideoFrames;
typedef Fifo<VideoFramePtr> FifoVideo;
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);
typedef boost::shared_ptr<wxBitmap> wxBitmapPtr;

class VideoFrame
    :   public boost::noncopyable
    ,   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialization AND allocation.
    VideoFrame(VideoFrameType type, wxSize size, pts position, int repeat);

    /// Initialization without allocation. Used for empty frames. Then, allocation only
    /// needed when the data is needed for playback. During 'track combining' empty
    /// frames can be ignored. This avoids needless allocation.
    VideoFrame(VideoFrameType type, wxSize size, pts position);

    virtual ~VideoFrame();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getRepeat() const;      ///< \return the number of times this frame should be displayed
    void setRepeat(int repeat); ///< \param new number of times to show this frame (used at end of clips)

    pts getPts() const;
    void setPts(pts position);
    wxSize getSize() const;
    int getSizeInBytes() const;
    wxBitmapPtr getBitmap();    ///< \return this frame as a wxBitmap

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /// Virtual and not const due to override in EmptyFrame
    virtual DataPointer getData();

    LineSizePointer getLineSizes() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    AVFrame* mFrame;
    VideoFrameType mType;
    int mRepeat;
    double mTimeStamp;
    wxSize mSize;
    pts mPts;
    boost::uint8_t *mBuffer;
    int mBufferSize;

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const VideoFrame& obj);
};

} // namespace

#endif // VIDEO_FRAME_H