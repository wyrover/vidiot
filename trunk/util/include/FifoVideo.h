#ifndef FIFO_VIDEO_H
#define FIFO_VIDEO_H

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
    VideoFrame(PixelFormat format, int width, int height, boost::int64_t pts, AVRational timebase, int repeat);
    virtual ~VideoFrame();

    /** uint8_t* data[4] */
    DataPointer getData() const;

    /** int linesize[4] */
    LineSizePointer getLineSizes() const;

    int getRepeat() const;
    boost::int64_t getPts() const; 
    void setPts(boost::int64_t pts);
    AVRational getTimeBase() const;
    double getTime() const;
    int getWidth() const;
    int getHeight() const;
    int getSizeInBytes() const;
private:
    AVFrame* mFrame;
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

#endif // FIFO_VIDEO_H