#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include <boost/shared_ptr.hpp>
#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilRTTI.h"

#pragma warning ( disable : 4005 ) // Redefinition of INTMAX_C/UINTMAX_C by boost and ffmpeg 

/** /todo replace all ffmpeg types with regular types */

extern "C" {
#include <avformat.h>
};

namespace model {

typedef boost::uint8_t** DataPointer;
typedef int* LineSizePointer;
class VideoFrame;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef std::list<VideoFramePtr> VideoFrames;
typedef Fifo<VideoFramePtr> FifoVideo;
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);

class VideoFrame
    :   public boost::noncopyable
    ,   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialization AND allocation.
    VideoFrame(PixelFormat format, int width, int height, pts position, AVRational timebase, int repeat);

    /// Initialization without allocation. Used for empty frames. Then, allocation only
    /// needed when the data is needed for playback. During 'track combining' empty 
    /// frames can be ignored. This avoids needless allocation.
    VideoFrame(PixelFormat format, int width, int height, pts position, AVRational timebase);

    virtual ~VideoFrame();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getRepeat() const;      ///< @return the number of times this frame should be displayed
    void setRepeat(int repeat); ///< @param new number of times to show this frame (used at end of clips)

    pts getPts() const; 
    void setPts(pts position);
    AVRational getTimeBase() const;
    double getTime() const;
    int getWidth() const;
    int getHeight() const;
    int getSizeInBytes() const;

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
    PixelFormat mFormat;
    int mRepeat;
    double mTimeStamp;
    int mWidth;
    int mHeight;
    pts mPts;
    AVRational mTimeBase;
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