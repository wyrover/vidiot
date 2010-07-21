#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include "UtilFifo.h"
#include <boost/cstdint.hpp>

#pragma warning ( disable : 4005 ) // Redefinition of INTMAX_C/UINTMAX_C by boost and ffmpeg 

/** /todo replace all ffmpeg types with regular types */

extern "C" {
#include <avformat.h>
};

typedef boost::uint8_t** DataPointer;
typedef int* LineSizePointer;

class VideoFrame : boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /**
    * Initialization and allocation.
    */
    VideoFrame(PixelFormat format, int width, int height, boost::int64_t pts, AVRational timebase, int repeat);

    /**
    * Initialization without allocation. Used for empty frames. Then, allocation only
    * needed when the data is needed for playback. During 'track combining' empty 
    * frames can be ignored. This avoids needless allocation.
    */
    VideoFrame(PixelFormat format, int width, int height, boost::int64_t pts, AVRational timebase);

    virtual ~VideoFrame();

    //////////////////////////////////////////////////////////////////////////
    // META DATA
    //////////////////////////////////////////////////////////////////////////

    int getRepeat() const;
    boost::int64_t getPts() const; 
    void setPts(boost::int64_t pts);
    AVRational getTimeBase() const;
    double getTime() const;
    int getWidth() const;
    int getHeight() const;
    int getSizeInBytes() const;

    template <typename Derived>
    bool isA()
    {
        return (typeid(Derived) == typeid(*this));
    }

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /**
    * uint8_t* data[4] 
    * Virtual and not const due to override in EmptyFrame
    */
    virtual DataPointer getData();

    /** int linesize[4] */
    LineSizePointer getLineSizes() const;

protected:
    AVFrame* mFrame;
    PixelFormat mFormat;
    int mRepeat;
    double mTimeStamp;
    int mWidth;
    int mHeight;
    boost::int64_t mPts;
    AVRational mTimeBase;
    boost::uint8_t *mBuffer;
    int mBufferSize;
};

typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef Fifo<VideoFramePtr> FifoVideo;

std::ostream& operator<< (std::ostream& os, const VideoFrame& obj);
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);

#endif // VIDEO_FRAME_H